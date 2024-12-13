# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -g

# Libraries
LIBS = -lssl -lcrypto

# UnitTest++ library
UNITTEST_LIBS = -lUnitTest++

# Source files for the main executable
SRCS = main.cpp Calculator.cpp ClientCommunicate.cpp Interface.cpp Error.cpp ConnectToBase.cpp


# Source files for tests
TEST_SRCS = tests.cpp Calculator.cpp ClientCommunicate.cpp Interface.cpp Error.cpp ConnectToBase.cpp


# Header files
HDRS = Calculator.h ClientCommunicate.h Interface.h Error.h ConnectToBase.h

# Executable name
EXECUTABLE = server

# Test executable name
TEST_EXECUTABLE = tests

# Default target
all: $(EXECUTABLE)

# Rule to compile the main program
$(EXECUTABLE): $(SRCS) $(HDRS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(EXECUTABLE) $(LIBS)

# Rule to compile the test program
test: $(TEST_SRCS) $(HDRS)
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) -o $(TEST_EXECUTABLE) $(LIBS) $(UNITTEST_LIBS)

# Rule to run tests
run_test: test
	./$(TEST_EXECUTABLE)

# Clean rule
clean:
	rm -f $(EXECUTABLE) $(TEST_EXECUTABLE)

.PHONY: all test run_test clean
