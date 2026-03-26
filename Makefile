# `make animate.o` compiles all library code into a single object file.

CC     = gcc
CFLAGS = -fPIC -Wall -Wvla -Werror -fsanitize=address -g
HEADERS = animate.h animate_internal.h

default: animate.o test_simple

animate.o: animate.c $(HEADERS)
	$(CC) $(CFLAGS) -c animate.c -o $@

test_simple: main_simple.c animate.o | animate.h
	$(CC) -fPIC -fsanitize=address -g $^ -o $@

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
	rm -f animate.o test_simple simple.dat
	rm -f Doxyfile
	rm -rf latex

clobber: clean
	rm -f $(API_DOC)

.PHONY: doc clean clobber default test
