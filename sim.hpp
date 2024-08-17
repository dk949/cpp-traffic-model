#ifndef SIM_HPP
#define SIM_HPP

#include <array>
#include <cassert>
#include <cmath>
#include <functional>
#include <stack>
#include <vector>

#ifdef NDEBUG
#    define DEBUG_PRINT(...)
#else
#    define DEBUG_PRINT(...) fmt::println(__VA_ARGS__)
#endif  // NDEBUG

#ifndef SIM_UNREACHABLE
#    define SIM_UNREACHABLE __builtin_unreachable
#else
#    error "SIM_UNREACHABLE" already defined
#endif

constexpr auto VOID_DEST = ~0ull;
#ifndef X
#    define X 81
#endif
#ifndef Y
#    define Y 41
#endif
constexpr auto NUM_CARS = 3;

[[nodiscard]]
constexpr std::size_t acc(int row, int col) {
    return col + row * X;
}

[[nodiscard]]
constexpr std::pair<int, int> splt(std::size_t idx) {
    return {idx / X, idx % X};
}


enum struct CellType { building, road };

enum Dir : unsigned { N = 0b0001, S = 0b0010, E = 0b0100, W = 0b1000 };

struct Car {
    bool operator==(Car const &) const = default;
    std::size_t dest = VOID_DEST;
    Dir next;
    int wait_ctr = 0;
    bool has_moved = false;
    int id = -1;
};

static_assert(sizeof(Car) == 24);

struct Cell {
    bool operator==(Cell const &) const = default;
    CellType type;
    unsigned dir;
    Car car = {.dest = VOID_DEST};

    [[nodiscard]]
    inline bool isIntersection() const {
        return !std::has_single_bit(dir);
    }

    [[nodiscard]]
    inline bool hasCar() const {
        return car.dest != VOID_DEST;
    }

    inline void removeCar() {
        car.dest = VOID_DEST;
    }
};

[[nodiscard]]
constexpr Dir rotateRight(Dir dir) {
    switch (dir) {
        case N: return W;
        case E: return N;
        case S: return E;
        case W: return S;
        default: SIM_UNREACHABLE();
    }
}

[[nodiscard]]
inline std::pair<int, int> cellAhead(int row, int col, Dir dir) {
    switch (dir) {
        case N: return {row - 1, col};
        case S: return {row + 1, col};
        case E: return {row, col + 1};
        case W: return {row, col - 1};
        default: assert(false); SIM_UNREACHABLE();
    }
}

[[nodiscard]]
inline std::pair<int, int> cellBehind(int row, int col, Dir dir) {
    switch (dir) {
        case N: return {row + 1, col};
        case S: return {row - 1, col};
        case E: return {row, col - 1};
        case W: return {row, col + 1};
        default: assert(false); SIM_UNREACHABLE();
    }
}

[[nodiscard]]
inline std::pair<Dir, Dir> intoDirs(unsigned dir) {
    std::array<Dir, 2> out {};
    int idx = 0;

    if (dir & N) out[idx++] = N;
    if (dir & S) out[idx++] = S;
    if (dir & E) out[idx++] = E;
    if (dir & W) out[idx++] = W;

    assert(out[0]);
    assert(out[1]);

    return {out[0], out[1]};
}

[[nodiscard]]
inline std::size_t manhattan(int row, int col, int dest_row, int dest_col) {
    return std::size_t(std::abs(row - dest_row) + std::abs(col - dest_col));
}

using Grid = std::vector<Cell>;
using Node = size_t;
using Path = std::stack<Node>;

Path aStar(Node start, Node goal, Grid const &grid);

#endif  // SIM_HPP
