CXX = g++
CXXFLAGS = -Wall -std=c++11 -I/usr/include -I/usr/include/UnitTest++ -DUNIT_TEST
LDFLAGS = -L/usr/lib/aarch64-linux-gnu -L/usr/local/lib -lUnitTest++ -lssl -lcrypto
SRC = tests.cpp Calculator.cpp ConnectToBase.cpp Error.cpp Interface.cpp ClientCommunicate.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)

