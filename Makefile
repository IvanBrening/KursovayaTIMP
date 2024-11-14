# Makefile
CXX = g++
CXXFLAGS = -Wall -std=c++11 -I/usr/include -I/usr/include/UnitTest++ -DUNIT_TEST
LDFLAGS = -L/usr/lib/aarch64-linux-gnu -L/usr/local/lib -lUnitTest++ -lssl -lcrypto

# Пути
UNIT_TEST_PATH = UnitTest
SERVER_PATH = server

# Файлы для тестов и сервера
SRC_TEST = $(UNIT_TEST_PATH)/tests.cpp $(UNIT_TEST_PATH)/Calculator.cpp $(UNIT_TEST_PATH)/ConnectToBase.cpp \
           $(UNIT_TEST_PATH)/Error.cpp $(UNIT_TEST_PATH)/Interface.cpp $(UNIT_TEST_PATH)/ClientCommunicate.cpp
TARGET_TEST = test

SRC_SERVER = $(SERVER_PATH)/server.cpp
TARGET_SERVER = server_app

# Сборка сервера
all: $(TARGET_SERVER)

$(TARGET_SERVER): $(SRC_SERVER)
	$(CXX) $(CXXFLAGS) $(SRC_SERVER) -o $(TARGET_SERVER) $(LDFLAGS)

# Сборка и запуск тестов
test: $(SRC_TEST)
	$(CXX) $(CXXFLAGS) $(SRC_TEST) -o $(TARGET_TEST) $(LDFLAGS)
	./$(TARGET_TEST)

# Очистка
clean:
	rm -f $(TARGET_TEST) $(TARGET_SERVER)

