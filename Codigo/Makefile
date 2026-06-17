CXX = g++
CXXFLAGS = -Wall -Wextra -O3 -std=c++11
TARGET = arsp
SRC = main.cpp funciones.cpp
HEADERS = estructuras.h

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)