# You may modify this file as you wish, but `make animate.o` should always
# compile all necessary code into a single object file.

CC     = gcc
CFLAGS = -fPIC -Wall -Wvla -Werror -fsanitize=address -g

HEADERS = animate.h animate_internal.h

# Source files that make up the library
LIB_SRCS = animate.c canvas.c sprite.c placement.c frame.c

# Intermediate objects (prefixed to avoid clashing with final animate.o)
INT_OBJS = $(patsubst %.c,._int_%.o,$(LIB_SRCS))

default: animate.o test_simple

# Combine all intermediate objects into a single relocatable object
animate.o: $(INT_OBJS)
	ld -r -o $@ $^

# Compile each library source to an intermediate object
._int_%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

test_simple: main_simple.c animate.o | animate.h
	$(CC) -fPIC -fsanitize=address -g $^ -o $@

# Tests
.PHONY: test
test: animate.o
	@cd tests && bash run_all_tests.sh

API_DOC = PointerProAnimateRefman.pdf
doc: $(API_DOC)

Doxyfile:
	doxygen -g
	sed -i 's/\(GENERATE_HTML *= *\).*/\1NO/g' $@
	sed -i 's/\(EXTRACT_ALL *= *\).*/\1YES/g' $@
	sed -i 's/\(INPUT *= *\).*/\1"animate.h"/g' $@
	sed -i 's/\(PROJECT_NAME *= *\).*/\1"PointerPro Animate"/g' $@
	sed -i 's/\(OPTIMIZE_OUTPUT_FOR_C *= *\).*/\1YES/g' $@

$(API_DOC): DOC_MAKEFILE=latex/Makefile
$(API_DOC): DOC_TOP=latex/refman.tex
$(API_DOC): Doxyfile | animate.h
	doxygen $^
	sed -i 's/\t$$(MKIDX/\t#/g'             $(DOC_MAKEFILE)
	sed -i 's/^ *\\clearemptydoublepage//g' $(DOC_TOP)
	sed -i 's/^ *\\tableofcontents//g'      $(DOC_TOP)
	sed -i 's/^ *\\chapter{File Index}//g'  $(DOC_TOP)
	sed -i 's/^ *\\input{files}//g'         $(DOC_TOP)
	cd latex && make
	cp latex/refman.pdf $@

clean:
	rm -f animate.o test_simple
	rm -f ._int_*.o
	rm -f Doxyfile
	rm -rf latex

clobber: clean
	rm -f $(API_DOC)

.PHONY: doc clean clobber default test
