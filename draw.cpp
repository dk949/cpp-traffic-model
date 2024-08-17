#include "draw.hpp"

#include <fmt/color.h>
#include <fmt/format.h>

#include <thread>
#ifndef FPS
#    define FPS 25
#endif
using namespace std::chrono_literals;

extern std::vector<Path> paths;

void drawGrid(Grid const &grid) {
    static std::string buf;
    buf.clear();
    static auto dir_to_arrow = [](unsigned dir) {
        switch (dir) {
            case N: return "🠕";
            case S: return "🠗";
            case E: return "🠖";
            case W: return "🠔";
            case E | N: return "⮥";
            case S | E: return "⮧";
            case N | W: return "⮤";
            case W | S: return "⮦";
            default: assert(false);
        }
    };
    static auto building = "■";
    static auto car = "●";
    for (int row = 0; row < Y; row++) {
        for (int col = 0; col < X; col++) {
            auto const &cell = grid[acc(row, col)];
            if (cell.type == CellType::building) {
                fmt::format_to(std::back_inserter(buf), fg(fmt::color::gray), "{} ", building);
                continue;
            }
            if (!cell.hasCar()) {
                fmt::format_to(std::back_inserter(buf), "{} ", dir_to_arrow(cell.dir));
                continue;
            }
            auto const color = [&]() {
                if (paths[cell.car.id].empty()) return fg(fmt::color::green);
                switch (cell.car.wait_ctr) {
                    case 0: return fg(fmt::color::blue);
                    case 1: return fg(fmt::color::yellow);
                    case 2: return fg(fmt::color::orange);
                    default: return fg(fmt::color::red);
                }
            }();
            fmt::format_to(std::back_inserter(buf), color, "{} ", car);
        }
        buf.push_back('\n');
    }
    buf.push_back('\n');

    std::fputs("\033[2J", stdout);
    std::fputs(buf.c_str(), stdout);
    std::this_thread::sleep_for(std::chrono::microseconds(size_t((1.0 / double(FPS)) * 1'000'000)));
}
