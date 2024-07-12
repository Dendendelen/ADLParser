
CFLAGS = -g

main: main.o node.o lexer.o parser.o exceptions.o
	g++ $(CFLAGS) -g -o main main.o node.o lexer.o parser.o exceptions.o

main.o: main.cpp lexer.h
	g++ $(CFLAGS) -o main.o -c main.cpp

node.o: node.cpp node.h
	g++ $(CFLAGS) -o node.o -c node.cpp

lexer.o: lexer.cpp lexer.h 
	g++ $(CFLAGS) -o lexer.o -c lexer.cpp

parser.o: parser.cpp parser.h
	g++ $(CFLAGS) -o parser.o -c parser.cpp

exceptions.o: exceptions.cpp exceptions.h
	g++ $(CFLAGS) -o exceptions.o -c exceptions.cpp

.PHONY: clean dot
clean:
	rm -rf *.o main

dot:
	dot -T png -O graph.gv