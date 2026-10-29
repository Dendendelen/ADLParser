#CFLAGS2 :=-g -fsanitize=address
ROOT_DIR :=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
CFLAGS :=-std=c++20 -g -Isrc/include -D'ROOT_DIR="$(ROOT_DIR)"'
SRCDIR :=src/
INCDIR :=src/include/
ODIR_NAME=out
ODIR :=$(ODIR_NAME)/

HEADERS_LIST :=node lexer parser exceptions alil alil_converter timber_converter coffea_converter ast_visitor config 
SOURCES_LIST :=main $(HEADERS_LIST) 

ALL_HEADERS := $(foreach x, $(HEADERS_LIST), $(INCDIR)$(x).hpp)

adlparser: $(foreach obj, $(SOURCES_LIST), $(ODIR)$(obj).o)
	g++ $(CFLAGS) -g -o adlparser $^
	./adlparser _ genconfig

define ALL_OUTS
$$(ODIR)$(1).o: $$(SRCDIR)$(1).cpp $$(ALL_HEADERS)
	mkdir -p $$(ODIR_NAME)
	g++ $$(CFLAGS) -o $$(ODIR)$(1).o -c $$(SRCDIR)$(1).cpp
endef

$(foreach item, $(SOURCES_LIST), $(eval $(call ALL_OUTS,$(item))))


$(ODIR_NAME):
	mkdir $(ODIR_NAME)

.PHONY: clean dot
clean:
	rm -rf $(ODIR_NAME)/*.o adlparser

dot:
	dot -T png -O graph.gv