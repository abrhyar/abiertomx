CXX = g++
CXXFLAGS = -std=c++17 -Wall -I.
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Isrc
LDFLAGS = -lncurseswSRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)
TARGET = alkall: $(TARGET)$(TARGET):$(OBJS)
$(CXX)$(OBJS) -o $(TARGET)$(LDFLAGS)%.o: %.cpp
$(CXX)$(CXXFLAGS) -c $< -o$@clean:
rm -f src/*.o $(TARGET).PHONY: all clean