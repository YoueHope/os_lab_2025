# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -O2
LDFLAGS = -lpthread

# Файлы
COMMON_SRC = common.c
COMMON_OBJ = common.o
CLIENT_SRC = client.c
SERVER_SRC = server.c
TARGET_CLIENT = client
TARGET_SERVER = server

# Цели по умолчанию
all: $(TARGET_CLIENT) $(TARGET_SERVER)

# Общая библиотека
$(COMMON_OBJ): $(COMMON_SRC) common.h
	$(CC) $(CFLAGS) -c -o $(COMMON_OBJ) $(COMMON_SRC)

# Клиент
$(TARGET_CLIENT): $(CLIENT_SRC) $(COMMON_OBJ) common.h
	$(CC) $(CFLAGS) -o $(TARGET_CLIENT) $(CLIENT_SRC) $(COMMON_OBJ) $(LDFLAGS)

# Сервер
$(TARGET_SERVER): $(SERVER_SRC) $(COMMON_OBJ) common.h
	$(CC) $(CFLAGS) -o $(TARGET_SERVER) $(SERVER_SRC) $(COMMON_OBJ) $(LDFLAGS)

# Статическая библиотека (альтернативный вариант)
libcommon.a: $(COMMON_OBJ)
	ar rcs libcommon.a $(COMMON_OBJ)

# Сборка со статической библиотекой
static: libcommon.a
	$(CC) $(CFLAGS) -o $(TARGET_CLIENT) $(CLIENT_SRC) -L. -lcommon $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(TARGET_SERVER) $(SERVER_SRC) -L. -lcommon $(LDFLAGS)

# Очистка
clean:
	rm -f $(TARGET_CLIENT) $(TARGET_SERVER) $(COMMON_OBJ) libcommon.a *.o

# Тестирование
test: all
	@echo "Starting servers..."
	@./server --port 20001 --tnum 2 &
	@./server --port 20002 --tnum 2 &
	@./server --port 20003 --tnum 2 &
	@sleep 2
	@echo "Testing client..."
	@./client --k 10 --mod 1000007 --servers servers.txt
	@pkill server || true

# Установка
install: all
	@echo "Installing to /usr/local/bin/"
	@sudo cp $(TARGET_CLIENT) /usr/local/bin/factorial-client
	@sudo cp $(TARGET_SERVER) /usr/local/bin/factorial-server

# Показать информацию
info:
	@echo "Common functions moved to library:"
	@echo "- MultModulo()"
	@echo "- Factorial()" 
	@echo "- ConvertStringToUI64()"
	@echo "- ReadServers()"
	@echo "- struct FactorialArgs"
	@echo "- struct Server"

.PHONY: all clean test install static info
