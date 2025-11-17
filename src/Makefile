CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -pedantic
OUTFILE := IFJcompiler
BUILDDIR := ../build
TESTDIR := ../tests
DOCDIR := ../doc
RMDIR := rm -rf

.PHONY: all test clean doc

all:
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -o $(BUILDDIR)/$(OUTFILE) *.c

test-symtable:
	mkdir -p $(BUILDDIR)
		$(CC) $(CFLAGS) -o $(BUILDDIR)/symtable_test symtable.c strutils.c $(TESTDIR)/test_symtable.c
	@echo "Running symtable tests..."
	$(BUILDDIR)/symtable_test

test-ast:
	mkdir -p $(BUILDDIR)
		$(CC) $(CFLAGS) -o $(BUILDDIR)/ast_test ast.c strutils.c $(TESTDIR)/test_ast.c
	@echo "Running AST tests..."
	$(BUILDDIR)/ast_test

test-scanner:
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -o $(BUILDDIR)/$(OUTFILE) *.c
	@echo "Running scanner tests..."
	../scanner_tests/run_scan_tests.sh > ../scanner_tests/scan_test_results.txt
	
doc: doxygen Doxyfile

clean:
	$(RMDIR) $(BUILDDIR)
	$(RMDIR) $(DOCDIR)
