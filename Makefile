CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Isrc
LDFLAGS = -lncursesw

SRCS = src/camera.cpp src/gallery.cpp src/games.cpp src/main.cpp src/utils.cpp src/web.cpp
OBJS = src/camera.o src/gallery.o src/games.o src/main.o src/utils.o src/web.o
TARGET = alk

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean