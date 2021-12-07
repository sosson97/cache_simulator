CXX=g++
CXXFLAGS=-I. -O2
#CXXFLAGS+=-g
#DEPS= cache_test.h
OBJ= belady.o lru.o cache_test.o twolevellru.o

cache_sim: LINK.o=$(LINK.cpp)
cache_sim: $(OBJ)

#%.o: %.c $(DEPS)
#	$(CC) $(CFLAGS) -c -o $@ $< 

cache_sim: $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS)

clean:
	rm *.o cache_sim

.PHONY:clean
