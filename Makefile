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

# Doxygen configuration
DOXYGEN_CONF = conf
DOCS_DIR = docs

# Default target
all: $(EXECUTABLE)

# Rule to compile the main program
$(EXECUTABLE): $(SRCS) $(HDRS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(EXECUTABLE) $(LIBS)

# Rule to compile and run tests
test:
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) -o $(TEST_EXECUTABLE) $(LIBS) $(UNITTEST_LIBS)
	./$(TEST_EXECUTABLE)

# Rule to generate documentation
doc:
	@echo "Генерация документации с использованием Doxygen..."
	@doxygen $(DOXYGEN_CONF)

# Rule to clean all generated files
clean:
	@echo "Удаление сгенерированных файлов..."
	@rm -f $(EXECUTABLE) $(TEST_EXECUTABLE)
	@rm -rf $(DOCS_DIR)

# .PHONY targets
.PHONY: all test doc clean
