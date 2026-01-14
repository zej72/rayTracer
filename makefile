CXX = g++
CXXFLAGS = -Wall -Wextra -O2

MAKEFLAGS += -j$(shell nproc 2>/dev/null || echo 4)

SRCS = main.cpp rayTracer.cpp misc.cpp objects.cpp Vec3.cpp
OBJS = $(SRCS:%.cpp=build/%.o)
TARGET = rayTracer
OUT = build

$(TARGET): $(OBJS)
	mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) $^ -o $(OUT)/$@

build/%.o: %.cpp
	mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OUT)

.PHONY: clean
