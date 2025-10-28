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

test:
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -o $(BUILDDIR)/symtable_test symtable.c $(TESTDIR)/test_symtable.c
	@echo "Running symtable tests..."
	$(BUILDDIR)/symtable_test

doc: doxygen Doxyfile

clean:
	$(RMDIR) $(BUILDDIR)
	$(RMDIR) $(DOCDIR)
