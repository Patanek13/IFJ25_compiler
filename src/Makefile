CC := gcc
CFLAGS := -Wall
OUTFILE := IFJcompiler
BUILDDIR := ../build
DOCDIR := ../doc
RMDIR := rm -rf

.PHONY: all test clean doc

all:
	$(CC) $(CFLAGS) -o $(BUILDDIR)/$(OUTFILE) *.c

doc: doxygen Doxyfile

clean:
	$(RMDIR) $(BUILDDIR)/*
	$(RMDIR) $(DOCDIR)
