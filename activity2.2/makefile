CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)
TARGET = activity2.2

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)