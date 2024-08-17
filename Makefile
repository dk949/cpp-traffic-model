ifdef DEBUG
OPT=-Og
LOPT=-lg
DRAW=draw.o
ASAN=-fsanitize=address
X=81
Y=41
FPS=7
else
OPT=-O3 -DNDEBUG
DRAW=draw_opt.o
X=1001
Y=1001
endif


CXXFLAGS=-std=c++20 $(ASAN) -g $(OPT) `pkg-config fmt --cflags` -Wall -Wextra -Wpedantic -Wno-missing-field-initializers -DX=$(X) -DY=$(Y) -DFPS=$(FPS)
LDFLAGS=-lm $(ASAN) $(LOPT) `pkg-config fmt --libs`
CC=g++
CXX=g++

%.o: %.cpp sim.hpp draw.hpp Makefile
	$(CXX) $< $(CXXFLAGS) -c -o $@

sim: sim.o path.o $(DRAW)
	$(CXX) $^ $(LDFLAGS) -o $@

clean:
	rm -f *.o
