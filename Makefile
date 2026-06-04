CXX = g++
CXXFLAGS = -std=c++17 -Iinclude

SRC_DIR = src
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

TARGET = installer

all: $(TARGET)

$(TARGET):
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)