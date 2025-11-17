#!/bin/bash

EXE=../build/IFJcompiler   # your compiled scanner binary
TEST_DIR=../scanner_tests
PASS=0
FAIL=0

for infile in $TEST_DIR/*.in; do
    base=$(basename "$infile" .in)
    outfile="$TEST_DIR/$base.out"

    echo ""
    echo "Running $base..."

    $EXE -s < "$infile" > output.tmp

    if diff -q output.tmp "$outfile" >/dev/null; then
        echo "  ✓ PASS"
        PASS=$((PASS+1))
    else
        echo "  ✗ FAIL"
        echo "  Differences:"
        diff output.tmp "$outfile"
        FAIL=$((FAIL+1))
    fi
done

rm -f output.tmp

echo ""
echo "Summary: $PASS passed, $FAIL failed."