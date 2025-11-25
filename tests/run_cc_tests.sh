#!/usr/bin/env bash
# Requires environment variables: TEST_DIR, PROJECT_BIN, INTERPRETER_BIN, OUT_FILE

set -u

# --- KONFIGURACE ---
TEST_DIR="codegen_tests"
PROJECT_BIN="../build/IFJcompiler"
INTERPRETER_BIN="tools/ic25int-linux-x86_64"
OUT_FILE="tools/output.ifjcode25"

# --- BARVY ---
if [[ -t 1 ]]; then
    GREEN=$'\033[32m'
    RED=$'\033[31m'
    YELLOW=$'\033[33m'
    BLUE=$'\033[34m'
    BOLD=$'\033[1m'
    RESET=$'\033[0m'
else
    GREEN=""
    RED=""
    YELLOW=""
    BLUE=""
    BOLD=""
    RESET=""
fi

# --- KONTROLY ---
if [ ! -d "$TEST_DIR" ]; then
    echo "${RED}ERROR: TEST_DIR '$TEST_DIR' is not a directory.${RESET}" >&2
    exit 2
fi
if [ ! -x "$PROJECT_BIN" ]; then
    echo "${RED}ERROR: PROJECT_BIN '$PROJECT_BIN' not found or not executable.${RESET}" >&2
    exit 2
fi
if [ ! -x "$INTERPRETER_BIN" ]; then
    echo "${RED}ERROR: INTERPRETER_BIN '$INTERPRETER_BIN' not found or not executable.${RESET}" >&2
    exit 2
fi

# --- INICIALIZACE ---
fail_count=0
pass_count=0
skipped_count=0
declare -a failed_tests  # Pole pro uložení jmen selhaných testů

echo "${BOLD}Spouštím testy generátoru kódu...${RESET}"
echo "--------------------------------------------------------"

# --- HLAVNÍ SMYČKA ---
while IFS= read -r -d '' file; do
    base="$(basename "$file")"

    # Extrakce čísla očekávaného návratového kódu
    case "$base" in
        *.txt) noext="${base%.txt}" ;;
        *)
            ((skipped_count++))
            continue
            ;;
    esac

    expected_raw="${noext##*_}"

    # Validace suffixu
    if [ "$expected_raw" = "$noext" ] || ! [[ "$expected_raw" =~ ^[0-9]+$ ]]; then
        echo "${YELLOW}[SKIP]${RESET} $base (špatný formát názvu, chybí _<číslo>)"
        ((skipped_count++))
        continue
    fi

    # Převod na číslo (base 10)
    expected=$((10#$expected_raw))

    # --- 1. KOMPILACE ---
    # Přesměrování stderr do /dev/null, aby ladicí výpisy nerušily v terminálu
    # Pokud chcete vidět chyby překladače, odstraňte "2> /dev/null"
    if ! "$PROJECT_BIN" < "$file" > "$OUT_FILE" 2> /dev/null; then
        # Pokud překladač selže, uložíme jeho návratový kód
        compiler_rc=$?
        # Pokud jsme neočekávali chybu kompilace (např. test očekává 0, ale překladač spadl)
        # tak to považujeme za FAIL už tady.
        if [ "$expected" -ne "$compiler_rc" ] && [ "$compiler_rc" -ne 0 ]; then
             echo "${RED}[FAIL]${RESET} $base ${BOLD}(Chyba překladu: $compiler_rc, očekáváno: $expected)${RESET}"
             ((fail_count++))
             failed_tests+=("$base (Compiler error: $compiler_rc)")
             continue
        fi
    fi

    # --- 2. INTERPRETACE ---
    # Interpretovi pošleme prázdný vstup, aby se nezasekl na instrukci READ
    # Zachytáváme stdout i stderr interpretu, abychom viděli případné chyby
    inter_out=$("$INTERPRETER_BIN" "$OUT_FILE" < /dev/null 2>&1)
    rc=$?

    # --- 3. VYHODNOCENÍ ---
    if [ "$rc" -eq "$expected" ]; then
        echo "${GREEN}[PASS]${RESET} $base (rc=$rc)"
        ((pass_count++))
    else
        echo "${RED}[FAIL]${RESET} $base ${BOLD}(rc=$rc, očekáváno=$expected)${RESET}"
        # Volitelně: vypsat chybu interpretu, pokud test selhal
        # echo "       Důvod: $inter_out"
        ((fail_count++))
        failed_tests+=("$base (Got $rc, Expected $expected)")
    fi

done < <(find "$TEST_DIR" -maxdepth 1 -type f -print0 | sort -z)


# --- SOUHRN ---
echo "--------------------------------------------------------"
echo "${BOLD}SOUHRN VÝSLEDKŮ:${RESET}"

if [ "$fail_count" -eq 0 ] && [ "$pass_count" -gt 0 ]; then
    echo "${GREEN}Všechny testy prošly! ($pass_count)${RESET}"
    if [ "$skipped_count" -gt 0 ]; then
        echo "${YELLOW}Přeskočeno: $skipped_count${RESET}"
    fi
    exit 0
else
    echo "Celkem testů: $((pass_count + fail_count))"
    echo "${GREEN}Úspěšných:    $pass_count${RESET}"
    echo "${RED}Selhaných:    $fail_count${RESET}"

    if [ "$skipped_count" -gt 0 ]; then
        echo "${YELLOW}Přeskočeno:   $skipped_count${RESET}"
    fi

    if [ "$fail_count" -gt 0 ]; then
        echo ""
        echo "${BOLD}Seznam selhaných testů:${RESET}"
        for fail in "${failed_tests[@]}"; do
            echo " - ${RED}$fail${RESET}"
        done
        exit 1
    fi

    # Pokud nebyly nalezeny žádné testy
    if [ "$pass_count" -eq 0 ]; then
        echo "${YELLOW}Nebyly spuštěny žádné testy.${RESET}"
        exit 2
    fi
fi
