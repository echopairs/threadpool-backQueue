CPPFLAGS=-Wall -g -m64 -std=c++11
CC=g++
LDFLAGS=-lpthread
main:taskQueue.o main.o
	$(CC) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)
main.o:main.cc
	$(CC) $(CPPFLAGS) -c $^
taskQueue.o:taskQueue.cc
	$(CC) $(CPPFLAGS) -c $^
clean:
	rm -rf main *.o 
