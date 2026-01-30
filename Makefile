CFLAGS2 = -g -fsanitize=address
CFLAGS = -g -Isrc/include
SRCDIR = src/
INCDIR = src/include/
ODIR = out/

main: $(ODIR)main.o $(ODIR)node.o $(ODIR)lexer.o $(ODIR)parser.o $(ODIR)exceptions.o $(ODIR)ali_converter.o $(ODIR)timber_converter.o $(ODIR)coffea_converter.o $(ODIR)ast_visitor.o $(ODIR)config.o
	g++ $(CFLAGS) -g -o main $(ODIR)main.o $(ODIR)node.o $(ODIR)lexer.o $(ODIR)parser.o $(ODIR)exceptions.o $(ODIR)ali_converter.o $(ODIR)timber_converter.o $(ODIR)coffea_converter.o $(ODIR)ast_visitor.o $(ODIR)config.o

$(ODIR)main.o: $(SRCDIR)main.cpp $(INCDIR)lexer.hpp 
	mkdir -p out
	g++ $(CFLAGS) -o $(ODIR)main.o -c $(SRCDIR)main.cpp

$(ODIR)node.o: $(SRCDIR)node.cpp $(INCDIR)node.hpp
	mkdir -p out
	g++ $(CFLAGS) -o $(ODIR)node.o -c $(SRCDIR)node.cpp

$(ODIR)lexer.o: $(SRCDIR)lexer.cpp $(INCDIR)lexer.hpp
	mkdir -p out
	g++ $(CFLAGS) -o $(ODIR)lexer.o -c $(SRCDIR)lexer.cpp

$(ODIR)parser.o: $(SRCDIR)parser.cpp $(INCDIR)parser.hpp
	mkdir -p out
	g++ $(CFLAGS) -o $(ODIR)parser.o -c $(SRCDIR)parser.cpp

$(ODIR)ast_visitor.o: $(SRCDIR)ast_visitor.cpp $(INCDIR)ast_visitor.hpp
	mkdir -p out
	g++ $(CFLAGS) -o $(ODIR)ast_visitor.o -c $(SRCDIR)ast_visitor.cpp

$(ODIR)ali_converter.o: $(SRCDIR)ali_converter.cpp $(INCDIR)ali_converter.hpp
	mkdir -p out
	g++ $(CFLAGS) -o $(ODIR)ali_converter.o -c $(SRCDIR)ali_converter.cpp

$(ODIR)timber_converter.o: $(SRCDIR)timber_converter.cpp $(INCDIR)timber_converter.hpp
	mkdir -p out
	g++ $(CFLAGS) -o $(ODIR)timber_converter.o -c $(SRCDIR)timber_converter.cpp

$(ODIR)coffea_converter.o: $(SRCDIR)coffea_converter.cpp $(INCDIR)coffea_converter.hpp
	mkdir -p out
	g++ $(CFLAGS) -o $(ODIR)coffea_converter.o -c $(SRCDIR)coffea_converter.cpp

$(ODIR)exceptions.o: $(SRCDIR)exceptions.cpp $(INCDIR)exceptions.hpp
	mkdir -p out
	g++ $(CFLAGS) -o $(ODIR)exceptions.o -c $(SRCDIR)exceptions.cpp

$(ODIR)config.o: $(SRCDIR)config.cpp $(INCDIR)config.hpp
	mkdir -p out
	g++ $(CFLAGS) -o $(ODIR)config.o -c $(SRCDIR)config.cpp

out:
	mkdir out

.PHONY: clean dot
clean:
	rm -rf out/*.o main

dot:
	dot -T png -O graph.gv