CC = gcc
CFLAGS = -Wall -Wextra

all: parallel_min_max process_memory zombie parallel_sum

# сборка программы из задания 1
parallel_min_max: parallel_min_max.o utils.o find_min_max.o
	$(CC) $(CFLAGS) parallel_min_max.o utils.o find_min_max.o -o parallel_min_max

parallel_min_max.o: parallel_min_max.c
	$(CC) $(CFLAGS) -c parallel_min_max.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

find_min_max.o: find_min_max.c find_min_max.h
	$(CC) $(CFLAGS) -c find_min_max.c

# сборка программы из задания 3
process_memory: process_memory.o
	$(CC) $(CFLAGS) process_memory.o -o process_memory

process_memory.o: process_memory.c
	$(CC) $(CFLAGS) -c process_memory.c

# сборка программы из задания 2
zombie: zombie.o
	$(CC) $(CFLAGS) zombie.o -o zombie

zombie.o: zombie.c
	$(CC) $(CFLAGS) -c zombie.c

# сборка статической библиотеки libsum.a
libsum.a: sum.o
	ar rcs libsum.a sum.o

sum.o: sum.c sum.h
	$(CC) $(CFLAGS) -c sum.c

# компиляция parallel_sum.c
parallel_sum.o: parallel_sum.c
	$(CC) $(CFLAGS) -c parallel_sum.c

# линковка исполняемого файла с библиотекой libsum.a и потоками
parallel_sum: parallel_sum.o utils.o libsum.a
	$(CC) $(CFLAGS) parallel_sum.o utils.o -L. -lsum -pthread -o parallel_sum

# очистка
clean:
	rm -f *.o *.a parallel_min_max process_memory zombie parallel_sum