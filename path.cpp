#include "sim.hpp"

#include <fmt/format.h>

#include <cassert>
#include <functional>
#include <limits>
#include <queue>
#include <stack>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

Path reconstruct_path(std::unordered_map<Node, Node> const &came_from, Node current) {
    auto total_path = Path {};
    total_path.push(current);
    while (came_from.contains(current)) {
        current = came_from.at(current);
        total_path.push(current);
    }
    return total_path;
}

double getScore(std::unordered_map<Node, double> const &score, Node node) {
    if (score.contains(node)) return score.at(node);
    return std::numeric_limits<double>::infinity();
}

// the node in open_set having the lowest f_score[] value
// This operation can occur in O(Log(N)) time if open_set is a min-heap or a priority queue
Node getCurrent(std::unordered_set<Node> const &open_set, std::unordered_map<Node, double> const &f_score) {
    auto score = std::numeric_limits<double>::infinity();
    auto out = VOID_DEST;
    for (auto const &n : open_set) {
        auto const new_score = getScore(f_score, n);
        if (new_score <= score) {
            score = new_score;
            out = n;
        }
    }
    assert(out != VOID_DEST);
    return out;
}

std::vector<Node> getNeighbours(Node current, Grid const &grid) {
    auto const [row, col] = splt(current);
    auto const &cell = grid[current];
    if (cell.type != CellType::road) {
        fmt::println("({}, {})", row, col);
        assert(false);
    }
    std::vector<Node> out;
    out.reserve(2);
    if (cell.isIntersection()) {
        auto const [d1, d2] = intoDirs(cell.dir);
        for (auto d : {d1, d2}) {
            auto const [new_row, new_col] = cellAhead(row, col, d);
            if (!(new_row < 0 || new_col < 0 || new_row >= Y || new_col >= X)) out.push_back(acc(new_row, new_col));
        }
    } else {
        auto const [new_row, new_col] = cellAhead(row, col, Dir(cell.dir));
        if (!(new_row < 0 || new_col < 0 || new_row >= Y || new_col >= X)) out.push_back(acc(new_row, new_col));
    }
    return out;
}

// d(current,neighbor) is the weight of the edge from current to neighbor
constexpr double d(Node, Node) {
    return 0.0;
}

double h(Node current, Node goal) {
    auto const [curr_row, curr_col] = splt(current);
    auto const [goal_row, gaol_col] = splt(goal);
    return double(manhattan(curr_row, curr_col, goal_row, gaol_col));
}

// A* finds a path from start to goal.
// h is the heuristic function. h(n) estimates the cost to reach goal from node n.
Path aStar(Node start, Node goal, Grid const &grid) {
    // The set of discovered nodes that may need to be (re-)expanded.
    // Initially, only the start node is known.
    // This is usually implemented as a min-heap or priority queue rather than a hash-set.
    std::unordered_set<Node> open_set {start};
    // For node n, came_from[n] is the node immediately preceding it on the cheapest path from the start
    // to n currently known.
    std::unordered_map<Node, Node> came_from {};
    // For node n, g_score[n] is the cost of the cheapest path from start to n currently known.
    std::unordered_map<Node, double> g_score {};
    g_score[start] = 0;
    // For node n, f_score[n] := g_score[n] + h(n). f_score[n] represents our current best guess as to
    // how cheap a path could be from start to finish if it goes through n.
    std::unordered_map<Node, double> f_score;
    f_score[start] = h(start, goal);
    while (!open_set.empty()) {
        auto current = getCurrent(open_set, f_score);
        if (current == goal) return reconstruct_path(came_from, current);
        open_set.erase(current);
        for (auto const &n : getNeighbours(current, grid)) {
            // tentative_g_score is the distance from start to the neighbor through current
            auto tentative_g_score = getScore(g_score, current) + d(current, n);
            auto const n_score = getScore(g_score, n);
            if (tentative_g_score < n_score) {
                // This path to neighbor is better than any previous one. Record it!
                came_from[n] = current;
                g_score[n] = tentative_g_score;
                f_score[n] = tentative_g_score + h(n, goal);
                if (!open_set.contains(n)) open_set.insert(n);
            }
        }
    }
    throw std::runtime_error("Open set is empty but goal was never reached");
}
