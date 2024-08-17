#include "sim.hpp"

#include "draw.hpp"

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <signal.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <numeric>
#include <stack>
#include <utility>
namespace chr = std::chrono;
using namespace std::chrono_literals;

std::vector<Path> paths;

void addCar(Node start, Node end, int id, Grid &grid) {
    auto &cell = grid[start];
    assert(!cell.isIntersection());
    assert(!cell.hasCar());
    cell.car = Car {.dest = end, .next = Dir(cell.dir), .id = id};
    assert(cell.car.id >= 0);
    assert(size_t(cell.car.id) == paths.size());
    paths.push_back(aStar(start, end, grid));
    paths.back().pop();
}

[[nodiscard]]
Grid mkgrid() {
    using enum CellType;
    Grid grid;
    for (int row = 0; row < Y; row++) {
        for (int col = 0; col < X; col++) {
            unsigned int dir = 0;
            if (col % 4 == 3) {
                dir |= N;
            } else if (col % 2 == 1) {
                dir |= S;
            }

            if (row % 4 == 3) {
                dir |= W;
            } else if (row % 2 == 1) {
                dir |= E;
            }
            if (!dir)
                grid.push_back({building});
            else
                grid.push_back({road, dir});
        }
    }

    auto ctr = [n = 0]() mutable {
        return n++;
    };


    addCar(acc(1, 0), acc(Y - 4, X - 1), ctr(), grid);
    addCar(acc(Y - 2, X - 1), acc(3, 0), ctr(), grid);


    for (int row = 1; row < X - 4; row += 4)
        addCar(acc(0, row), acc(Y - 1, row + 4), ctr(), grid);

    for (int row = 3; row < X - 4; row += 4)
        addCar(acc(Y - 1, row + 4), acc(0, row), ctr(), grid);


    return grid;
}

[[nodiscard]]
bool canMoveTo(int row, int col, Grid const &grid, Dir dir) {
    // Car has hit a wall
    if (row < 0 || row >= Y || col < 0 || col >= X) {
        DEBUG_PRINT("can't move because out of bounds");
        return false;
    }
    auto const rotated = rotateRight(dir);
    auto &cell = grid[acc(row, col)];

    // Cell ahead has a car. Keep a safe distance :)
    if (cell.hasCar()) {
        DEBUG_PRINT("can't move because has car");
        return false;
    }


    // Car has right-of-way or it's not an intersection
    if (!(cell.dir & rotated)) return true;

    auto const [row_behind, col_behind] = cellBehind(row, col, rotated);
    // A car will be in this cell on the next turn
    if (grid[acc(row_behind, col_behind)].hasCar()) {
        DEBUG_PRINT("can't move because car will be here on next move");
        return false;
    }

    return true;
}

[[nodiscard]]
bool canMoveTo(Node from, Node to, Grid const &grid) {
    auto const [to_r, to_c] = splt(to);
    if (to_r < 0 || to_c < 0 || to_r >= Y || to_c >= X) {
        DEBUG_PRINT("Can't move: Out of bounds");
        return false;
    }
    auto const &cell_to = grid[to];
    auto const &cell_from = grid[from];
    if (cell_to.hasCar()) {
        DEBUG_PRINT("Can't move: occupied");
        return false;
    }
    if (cell_to.isIntersection()) {
        assert(!cell_from.isIntersection());
        // rigth of way
        if (cell_from.dir == E || cell_from.dir == W) return true;
        auto const other_dir = cell_to.dir & (~cell_from.dir);
        auto const [other_r, other_c] = cellBehind(to_r, to_c, Dir(other_dir));
        if (grid[acc(other_r, other_c)].hasCar()) {
            DEBUG_PRINT("Can't move: no right of way");
            return false;
        }
    }
    return true;
}

Dir relativeDir(Node from, Node to, Grid const &grid) {
    if (grid[from].isIntersection())
        return Dir(grid[to].dir);
    else
        return Dir(grid[from].dir);
}

bool calcNext(Grid &grid) {
    bool cars_need_to_move = false;
    for (int row = 0; row < Y; row++) {
        for (int col = 0; col < X; col++) {
            auto &cell = grid[acc(row, col)];
            cell.car.has_moved = false;
            if (!cell.hasCar()) continue;
            auto &path = paths[cell.car.id];
            if (canMoveTo(acc(row, col), path.top(), grid)) {
                cars_need_to_move = true;
                cell.car.wait_ctr = 0;
                cell.car.next = relativeDir(acc(row, col), path.top(), grid);
                path.pop();
            } else {
                cell.car.wait_ctr++;
            }
        }
    }
    return cars_need_to_move;
}

//
void makeMove(Grid &grid) {
    for (int row = 0; row < Y; row++) {
        for (int col = 0; col < X; col++) {
            auto &cell = grid[acc(row, col)];
            if (!cell.hasCar() || cell.car.has_moved) continue;
            // car is waiting
            if (cell.car.wait_ctr) continue;
            auto const [next_row, nex_col] = cellAhead(row, col, cell.car.next);
            auto &next_cell = grid[acc(next_row, nex_col)];
            assert(!next_cell.hasCar());
            assert(next_cell.type != CellType::building);
            next_cell.car = cell.car;
            next_cell.car.has_moved = true;
            cell.removeCar();
        }
    }
}

void removeFinishedCars(Grid &grid) {
    std::size_t idx = 0;
    for (auto it = grid.begin(); it != grid.end(); ++it, idx++) {
        assert(idx != VOID_DEST);
        if (it->car.dest == idx) it->removeCar();
    }
}

[[nodiscard]]
bool allCarsGone(Grid const &grid) {
    return std::transform_reduce(grid.begin(), grid.end(), true, std::logical_and {}, [](Cell const &cell) {
        return !cell.hasCar();
    });
}

int main() {
    assert((X - 1) % 4 == 0);
    assert((Y - 1) % 4 == 0);
    auto grid = mkgrid();
    drawGrid(grid);
    auto const start = chr::high_resolution_clock::now();
    for (int step = 0;; step++) {
        if (!calcNext(grid)) {
            fmt::println("Out of moves");
            break;
        }
        makeMove(grid);
        drawGrid(grid);
        removeFinishedCars(grid);
        if (allCarsGone(grid)) {
            auto const stop = chr::high_resolution_clock::now();
            auto const took = chr::duration_cast<chr::duration<double>>(stop - start);
            fmt::println("All cars have reached their destinations in {} steps. Took {}", step, took);
            break;
        }
    }
}
