CFLAGS2 = -g -fsanitize=address
CFLAGS = -g -Isrc/include
SRCDIR = src/
INCDIR = src/include/
ODIR = out/

main: $(ODIR)main.o $(ODIR)node.o $(ODIR)lexer.o $(ODIR)parser.o $(ODIR)exceptions.o $(ODIR)ali_converter.o $(ODIR)timber_converter.o $(ODIR)ast_visitor.o
	g++ $(CFLAGS) -g -o main $(ODIR)main.o $(ODIR)node.o $(ODIR)lexer.o $(ODIR)parser.o $(ODIR)exceptions.o $(ODIR)ali_converter.o $(ODIR)timber_converter.o $(ODIR)ast_visitor.o

out/main.o: $(SRCDIR)main.cpp $(INCDIR)lexer.hpp out
	g++ $(CFLAGS) -o $(ODIR)main.o -c $(SRCDIR)main.cpp

out/node.o: $(SRCDIR)node.cpp $(INCDIR)node.hpp out
	g++ $(CFLAGS) -o $(ODIR)node.o -c $(SRCDIR)node.cpp

out/lexer.o: $(SRCDIR)lexer.cpp $(INCDIR)lexer.hpp out
	g++ $(CFLAGS) -o $(ODIR)lexer.o -c $(SRCDIR)lexer.cpp

out/parser.o: $(SRCDIR)parser.cpp $(INCDIR)parser.hpp out
	g++ $(CFLAGS) -o $(ODIR)parser.o -c $(SRCDIR)parser.cpp

out/ast_visitor.o: $(SRCDIR)ast_visitor.cpp $(INCDIR)ast_visitor.hpp out
	g++ $(CFLAGS) -o $(ODIR)ast_visitor.o -c $(SRCDIR)ast_visitor.cpp

out/ali_converter.o: $(SRCDIR)ali_converter.cpp $(INCDIR)ali_converter.hpp out
	g++ $(CFLAGS) -o $(ODIR)ali_converter.o -c $(SRCDIR)ali_converter.cpp

out/timber_converter.o: $(SRCDIR)timber_converter.cpp $(INCDIR)timber_converter.hpp out
	g++ $(CFLAGS) -o $(ODIR)timber_converter.o -c $(SRCDIR)timber_converter.cpp

out/exceptions.o: $(SRCDIR)exceptions.cpp $(INCDIR)exceptions.hpp out
	g++ $(CFLAGS) -o $(ODIR)exceptions.o -c $(SRCDIR)exceptions.cpp

out:
	mkdir out

.PHONY: clean dot
clean:
	rm -rf out/*.o main

dot:
	dot -T png -O graph.gv