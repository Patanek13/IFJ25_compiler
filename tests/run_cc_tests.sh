#!/usr/bin/env bash
# run_tests.sh - run tests in TEST_DIR where filenames end in _<num>
# Requires environment variables: TEST_DIR, PROJECT_BIN, INTERPRETER_BIN, OUT_FILE

set -u

# === Colors ===
C_RED="\033[1;31m"
C_GREEN="\033[1;32m"
C_YELLOW="\033[1;33m"
C_BLUE="\033[1;34m"
C_RESET="\033[0m"

# required variables
TEST_DIR="codegen_tests"
PROJECT_BIN="../build/IFJcompiler"
INTERPRETER_BIN="./tools/ic25int-linux-x86_64"
OUT_FILE="./tools/output.ifjcode25"

# sanity checks
if [ ! -d "$TEST_DIR" ]; then
  echo -e "${C_RED}ERROR:${C_RESET} TEST_DIR '$TEST_DIR' is not a directory." >&2
  exit 2
fi
if [ ! -x "$PROJECT_BIN" ]; then
  echo -e "${C_RED}ERROR:${C_RESET} PROJECT_BIN '$PROJECT_BIN' not found or not executable." >&2
  exit 2
fi
if [ ! -x "$INTERPRETER_BIN" ]; then
  echo -e "${C_RED}ERROR:${C_RESET} INTERPRETER_BIN '$INTERPRETER_BIN' not found or not executable." >&2
  exit 2
fi

fail_count=0
processed_count=0
skipped_count=0

# find files (non-recursive), handle arbitrary filenames safely
while IFS= read -r -d '' file; do
  base="$(basename "$file")"

  # Only process files that end with .txt and have a suffix _<num> before .txt
  case "$base" in
    *.txt) noext="${base%.txt}" ;;
    *.out) continue ;;
    *)
      skipped_count=$((skipped_count + 1))
      continue
      ;;
  esac

  # expected exit code is the part after the last underscore in noext
  expected_raw="${noext##*_}"

  # if there was no underscore (no change), skip
  if [ "$expected_raw" = "$noext" ]; then
    echo -e "${C_YELLOW}SKIP:${C_RESET} '$base'   missing '_<num>.txt' suffix." >&2
    skipped_count=$((skipped_count + 1))
    continue
  fi

  # validate expected is a sequence of digits
  if ! [[ "$expected_raw" =~ ^[0-9]+$ ]]; then
    echo -e "${C_YELLOW}SKIP:${C_RESET} '$base'   suffix '_$expected_raw' is not a non-negative integer." >&2
    skipped_count=$((skipped_count + 1))
    continue
  fi

  # parse expected in base 10 to avoid octal interpretation on leading zeros
  expected=$((10#$expected_raw))

  processed_count=$((processed_count + 1))

  echo -e "${C_BLUE}----${C_RESET}"
  echo -e "${C_BLUE}TEST:${C_RESET} $base    expected exit: $expected"

  # Run project and save return code into rc
  "$PROJECT_BIN" < "$file" > "$OUT_FILE" 2> /dev/null
  rc=$?
  # Run project (redirect input -> OUT_FILE).
  if [ "$rc" -ne 0 ]; then
    echo -e "${C_YELLOW}NOTE:${C_RESET} ${PROJECT_BIN} exited non-zero for '$base'" >&2
  fi
  # if we returned 0, run the interpreter on the output file
  if [ "$rc" -eq 0 ]; then
    echo -e "${C_GREEN}PROJECT BIN OK:${C_RESET} exited 0. Continuing to interpreter."
    tmp_out="$(mktemp)"
    "$INTERPRETER_BIN" "$OUT_FILE" > "$tmp_out"
    rc=$?
  fi

  ############################################################
  # RC kontrola
  # - ak expected je 6 alebo 26   akceptujeme RC 6 aj 26
  # - ak expected je 5 alebo 25   akceptujeme RC 5 aj 25
  # - inak vy adujeme presn  match
  ############################################################
  rc_ok=0
  if [ "$expected" -eq 6 ] || [ "$expected" -eq 26 ]; then
    if [ "$rc" -eq 6 ] || [ "$rc" -eq 26 ]; then
      rc_ok=1
    fi
  elif [ "$expected" -eq 5 ] || [ "$expected" -eq 25 ]; then
    if [ "$rc" -eq 5 ] || [ "$rc" -eq 25 ]; then
      rc_ok=1
    fi
  else
    if [ "$rc" -eq "$expected" ]; then
      rc_ok=1
    fi
  fi

  if [ "$rc_ok" -eq 1 ]; then
    if [ "$expected" -eq 6 ] || [ "$expected" -eq 26 ]; then
      echo -e "${C_GREEN}RC  OK:${C_RESET} $rc (expected 6 or 26)"
    elif [ "$expected" -eq 5 ] || [ "$expected" -eq 25 ]; then
      echo -e "${C_GREEN}RC  OK:${C_RESET} $rc (expected 5 or 25)"
    else
      echo -e "${C_GREEN}RC  OK:${C_RESET} $rc"
    fi
  else
    if [ "$expected" -eq 6 ] || [ "$expected" -eq 26 ]; then
      echo -e "${C_RED}RC FAIL:${C_RESET} got $rc, expected 6 or 26"
    elif [ "$expected" -eq 5 ] || [ "$expected" -eq 25 ]; then
      echo -e "${C_RED}RC FAIL:${C_RESET} got $rc, expected 5 or 25"
    else
      echo -e "${C_RED}RC FAIL:${C_RESET} got $rc, expected $expected"
    fi
    fail_count=$((fail_count + 1))
    echo -e "${C_YELLOW}OUT SKIP:${C_RESET} RC mismatch, not comparing stdout."
    rm -f "$tmp_out"
    continue
  fi

  # Compare stdout with reference .out if it exists
  ref_out="${file%.txt}.out"
  if [ -f "$ref_out" ]; then
    if diff -u "$ref_out" "$tmp_out" > /dev/null; then
      echo -e "${C_GREEN}OUT OK${C_RESET}"
    else
      echo -e "${C_RED}OUT FAIL:${C_RESET} stdout differs from $ref_out"
      echo "---- diff ----"
      diff -u "$ref_out" "$tmp_out"
      fail_count=$((fail_count + 1))
    fi
  else
    echo -e "${C_YELLOW}OUT SKIP:${C_RESET} no expected output file '$ref_out'"
  fi

  rm -f "$tmp_out"

done < <(find "$TEST_DIR" -maxdepth 1 -type f -print0 | sort -z)

echo -e "${C_BLUE}----${C_RESET}"
if [ "$processed_count" -eq 0 ]; then
  echo -e "${C_RED}No test files matching '*_<num>.txt' were found in '$TEST_DIR'.${C_RESET}" >&2
  echo -e "${C_YELLOW}Skipped $skipped_count other files.${C_RESET}" >&2
  exit 2
fi

if [ "$fail_count" -eq 0 ]; then
  echo -e "${C_GREEN}All $processed_count tests passed (skipped $skipped_count files).${C_RESET}"
  exit 0
else
  echo -e "${C_RED}$fail_count of $processed_count tests failed (skipped $skipped_count files).${C_RESET}"
  exit 1
fi
