CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -pedantic -lm -g
OUTFILE := IFJcompiler
BUILDDIR := ../build
TESTDIR := ../tests
DOCDIR := ../doc
RMDIR := rm -rf
XLOGIN := xlostap00

.PHONY: all test clean doc

all:
	$(CC) $(CFLAGS) -o $(OUTFILE) *.c

dev:
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
	../scannertests/run_scan_tests.sh > ../scannertests/scan_test_results.txt
	../scannertests/run_lex_tests.sh > ../scannertests/lex_test_results.txt

doc: doxygen Doxyfile

pack:
	@echo "--- Creating archive $(XLOGIN).zip ---"
	@zip -j ../$(XLOGIN).zip ./* ../rozdeleni ../dokumentace.pdf ../rozsireni -x Doxyfile > /dev/null
	@echo "--- Running archive check ---"
	@mkdir -p ../TEST
	@../is_it_ok.sh ../$(XLOGIN).zip ../TEST > ../check.log
	@cat ../check.log
	@if grep -q "ERROR" ../check.log; then \
		echo "\n\033[1;31m✗ Archive check failed. Errors were found in the archive.\033[0m"; \
		exit 1; \
	else \
		echo "\n\033[1;32m✓ All checks passed successfully! The archive is ready for submission.\033[0m"; \
	fi

clean:
	$(RMDIR) $(BUILDDIR)
	$(RMDIR) $(DOCDIR)
	$(RMDIR) $(OUTFILE)
	$(RMDIR) ../$(XLOGIN).zip
	$(RMDIR) ../TEST
	$(RMDIR) ../check.log
