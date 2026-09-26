CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Isrc
LDFLAGS = -lSDL2 -lSDL2_ttf

SRC = src/main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = alk

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)