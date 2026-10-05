CXX = g++
CXXFLAGS = -std=c++20 -O2 \
	-Wall -Wextra -Wpedantic \
	-Wshadow -Wnon-virtual-dtor -Wold-style-cast \
	-Wcast-align -Wunused -Woverloaded-virtual \
	-Wconversion -Wsign-conversion -Wnull-dereference \
	-Wdouble-promotion -Wformat=2 \
	-Werror

.PHONY: all clean

all:
	$(CXX) $(CXXFLAGS) tests.cpp -o 2qcache.out

clean:
	rm -f *.out
