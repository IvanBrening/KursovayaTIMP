# Имя исполняемого файла
TARGET = server

# Компилятор и флаги
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2
LDFLAGS = -lssl -lcrypto

# Исходные файлы
SRC = server.cpp

# Цель по умолчанию
all: $(TARGET)

# Сборка целевого исполняемого файла напрямую из исходного
$(TARGET):
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Удаление исполняемого файла
clean:
	rm -f $(TARGET)

# Упрощение команд
.PHONY: all clean
