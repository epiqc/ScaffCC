CC=g++ -g -std=c++0x
OPTS=-O1
INC=-I./include

default: example

example: src/example.o src/charbits.o src/RotLib.o
	$(CC) $(OPTS) src/example.o src/charbits.o src/RotLib.o -o example

src/example.o: src/example.cpp include/RotLib.h
	echo $(INC)
	$(CC) $(OPTS) $(INC) -c src/example.cpp -o src/example.o

src/RotLib.o: src/RotLib.cpp
	$(CC) $(OPTS) $(INC) -c src/RotLib.cpp -o src/RotLib.o
	
src/charbits.o: src/charbits.cpp
	$(CC) $(OPTS) $(INC) -c src/charbits.cpp -o src/charbits.o


clean: 
	rm -f src/*.o example
