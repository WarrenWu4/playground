CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude -g
SRC = src/main.cpp
OBJ = build/main.o
EXE = build/main

all: $(EXE)

build:
	mkdir -p build

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXE): $(OBJ)
	$(CXX) $(OBJ) $(CXXFLAGS) -o $(EXE)

clean:
	rm -rf build
	rm log comp

perf: $(EXE)
	perf record -F 99 -g ./$(EXE)
	perf script > out.perf

run: $(EXE)
	./$(EXE) $(MESSAGES) $(KB)

.PHONY: all run clean
