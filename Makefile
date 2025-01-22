
CFLAGS = -g -fsanitize=address

main: main.o node.o lexer.o parser.o exceptions.o ali_converter.o timber_converter.o ast_visitor.o
	g++ $(CFLAGS) -g -o main main.o node.o lexer.o parser.o exceptions.o ali_converter.o timber_converter.o ast_visitor.o

main.o: main.cpp lexer.h
	g++ $(CFLAGS) -o main.o -c main.cpp

node.o: node.cpp node.h
	g++ $(CFLAGS) -o node.o -c node.cpp

lexer.o: lexer.cpp lexer.h 
	g++ $(CFLAGS) -o lexer.o -c lexer.cpp

parser.o: parser.cpp parser.h
	g++ $(CFLAGS) -o parser.o -c parser.cpp

ast_visitor.o: ast_visitor.cpp ast_visitor.h
	g++ $(CFLAGS) -o ast_visitor.o -c ast_visitor.cpp

ali_converter.o: ali_converter.cpp ali_converter.h
	g++ $(CFLAGS) -o ali_converter.o -c ali_converter.cpp

timber_converter.o: timber_converter.cpp timber_converter.h
	g++ $(CFLAGS) -o timber_converter.o -c timber_converter.cpp

exceptions.o: exceptions.cpp exceptions.h
	g++ $(CFLAGS) -o exceptions.o -c exceptions.cpp

.PHONY: clean dot
clean:
	rm -rf *.o main

dot:
	dot -T png -O graph.gv