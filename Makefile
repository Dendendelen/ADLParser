
main: main.o node.o lexer.o
	g++ -o main main.o node.o lexer.o

main.o: main.cpp lexer.h
	g++ -o main.o -c main.cpp

node.o: node.cpp node.h
	g++ -o node.o -c node.cpp

lexer.o: lexer.cpp lexer.h 
	g++ -o lexer.o -c lexer.cpp

.PHONY clean:
	rm -rf *.o main