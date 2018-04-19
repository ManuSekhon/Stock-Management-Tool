# Makefile

# compiler to use
CXX := clang++

# cpp flags 
CXXFLAGS := -Wall -Werror -Wextra -Wno-unused-parameter -O0 -std=c++14 -ggdb3

# libraries linked
LIB := -lsqlite3

# source files
SRC := main.cpp helpers.cpp

# header files
HDR := helpers.hpp

# compile program
main: $(HDR) $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o main $(LIB)

.PHONY: main