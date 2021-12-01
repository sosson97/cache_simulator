CC=g++
CFLAGS=-I -O2
DEPS= cache_test.h
OBJ= belady.o lru.o cache_test.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $ (CFLAGS)

cache_sim: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm *.o cache_sim
