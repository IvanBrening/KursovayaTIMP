# Компилятор
CXX = g++
CXXFLAGS = -Wall -g -std=c++11 -I/usr/include/UnitTest++  # Или используйте /usr/local/include/UnitTest++
LDFLAGS = -lssl -lcrypto -lUnitTest++

# Основные объектные файлы для сервера
OBJS = main.o Error.o Calculator.o ConnectToBase.o Interface.o ClientCommunicate.o
TARGET = server

# Объектные файлы для тестирования
TEST_OBJS = tests.o Error.o ClientCommunicate.o ConnectToBase.o Interface.o Calculator.o
TEST_TARGET = tests

# Стандартная цель для компиляции сервера
all: $(TARGET)

# Линковка исполняемого файла сервера
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Линковка исполняемого файла тестов
$(TEST_TARGET): $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) -o $(TEST_TARGET) $(LDFLAGS)

# Правило для компиляции .cpp файлов в .o
%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Цель для запуска тестов
run: $(TEST_TARGET)
	./$(TEST_TARGET)

# Очистка сгенерированных файлов
clean:
	rm -f $(OBJS) $(TARGET) $(TEST_OBJS) $(TEST_TARGET)

.PHONY: all clean run


