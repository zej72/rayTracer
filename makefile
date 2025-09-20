CXX = g++

SRCS = main.cpp rayTracer.cpp misc.cpp objects.cpp Vec3.cpp
TARGET = rayTracer
OUT = build

$(TARGET): $(OBJCS)
	mkdir -p $(OUT)
	$(CXX) $(SRCS) -o $(TARGET)

clean:
	rm $(TARGET)
