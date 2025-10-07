# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic
LDFLAGS = -lpthread

# Цели (исполняемые файлы)
TARGETS = parallel_min_max process_memory parallel_sum

# Основная цель - сборка всех программ
all: $(TARGETS)

# Сборка parallel_min_max с зависимостями
parallel_min_max: parallel_min_max.o find_min_max.o utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Сборка process_memory
process_memory: process_memory.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Сборка parallel_sum с зависимостями
parallel_sum: parallel_sum.o sum_lib.o utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Правила компиляции объектных файлов
parallel_min_max.o: parallel_min_max.c find_min_max.h utils.h
	$(CC) $(CFLAGS) -c parallel_min_max.c

find_min_max.o: find_min_max.c find_min_max.h
	$(CC) $(CFLAGS) -c find_min_max.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

process_memory.o: process_memory.c
	$(CC) $(CFLAGS) -c process_memory.c

parallel_sum.o: parallel_sum.c sum_lib.h utils.h
	$(CC) $(CFLAGS) -c parallel_sum.c

sum_lib.o: sum_lib.c sum_lib.h
	$(CC) $(CFLAGS) -c sum_lib.c

# Очистка собранных файлов
clean:
	rm -f $(TARGETS) *.o

# Пересборка
rebuild: clean all

# Установка
install: all
	@echo "Programs are built and ready in current directory"

# Справка
help:
	@echo "Available targets:"
	@echo "  all      - Build all programs (default)"
	@echo "  clean    - Remove all built files"
	@echo "  rebuild  - Clean and rebuild everything"
	@echo "  install  - Build programs (same as all)"
	@echo "  help     - Show this help"

# Объявляем фиктивные цели
.PHONY: all clean rebuild install help
