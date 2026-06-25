CXX = g++
CXXFLAGS = -Wall -Wextra -O3 -std=c++11
TARGET = arsp
SRC = main.cpp unipista.cpp multipista.cpp
HEADERS = estructuras.h

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	mkdir -p Output_Unipista
	mkdir -p Output_Multipista
	./$(TARGET)

clean:
	rm -f $(TARGET)