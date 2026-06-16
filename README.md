# IFJ25 Compiler

A complete, modular compiler for the imperative programming language **IFJ25** (a simplified subset of the [Wren](https://wren.io/) language). The compiler is implemented entirely in **C (C11 standard)**. This project forms the complete translation pipeline, mapping IFJ25 source code directly to the **IFJcode25** intermediate target language.

---

## 👥 Team Members (Variant TRP-izp)

| Member | Login | Share |
|--------|-------|-------|
| **Patrik Lošťák** | `xlostap00` | Team Leader (31%) |
| **Šimon Čorej** | `xcorejs00` | 23% |
| **Petr David Lanča** | `xlancap00` | 23% |
| **Sebastián Kuchta** | `xkuchts00` | 23% |

---

## 🛠️ Compiler Architecture

The project processes source code through a standard multi-pass compilation pipeline:

### 1. Lexical Analysis (`scanner.c`)

- Implemented as a **Finite State Machine (FSM)**.
- Reads standard input character-by-character and transforms it into a stream of tokens (`TokenType` and `TokenValue`).
- Supports sophisticated lexing features such as escape sequence translation, numeric parsing (floating-point with exponents, hexadecimal format), and efficient keyword recognition using lookup tables.

### 2. Syntax Analysis (`parser.c`)

The parser is the central controller of the compilation process, operating in two distinct modes to build an **Abstract Syntax Tree (AST)**:

- **Top-Down Recursive Descent (LL(1) Grammar):** Handles the main program skeleton, block bodies, control structures (`if`, `while`, `else`), and function definitions.
- **Precedence Analysis (Bottom-Up):** Dedicated exclusively to parsing complex arithmetic, logical, and relational expressions (`parse_expression()`). It utilizes a precedence table and a stack of AST nodes to resolve operator precedence and associativity seamlessly (`do_reduction()`).

### 3. Semantic Analysis (`semantic.c`)

Operates in **two passes** traversing the constructed AST:

- **Pass 1 (`PHASE_DEFINITION`):** Collects global symbols (user-defined functions, getters, setters) into the global Symbol Table and enforces redefinition rules.
- **Pass 2 (`PHASE_ANALYSIS`):** Validates function bodies, checks for undeclared variables, enforces type compatibility rules across expressions, and meticulously tracks local scope nesting boundaries and variable shadowing (utilizing name mangling where appropriate).

### 4. Code Generation (`generator.c`)

- Traverses the verified AST to generate equivalent instructions in the **IFJcode25** target language.
- Outputs the translated intermediate code to `stdout`.

---

## ⚙️ Requirements & Building

- **Environment:** GNU/Linux (tested extensively on Ubuntu)
- **Compiler:** `gcc` (C11 standard support)
- **Build System:** `make`
- **Testing:** `timeout` utility required for script execution
- **Documentation:** `doxygen` (optional)

### Building the Compiler

```bash
cd src
make
```

This produces the executable binary `IFJcompiler` in the `src/` directory.

> **Optional:** For an out-of-source debug build (generates binary in `/build` directory):
> ```bash
> cd src
> make dev
> ```

---

## 🚀 Usage

The compiler reads IFJ25 source code from `stdin` and writes the generated IFJcode25 instructions to `stdout`. Error messages and debug warnings are routed to `stderr`.

### Basic Execution

```bash
cd src
./IFJcompiler < ../samples/test.wren
```

### Debug Mode (`-d` flag)

```bash
cd src
./IFJcompiler -d < ../samples/test.wren
```

### Scanner-Only Validation (`-s` flag)

Prints the token stream without parsing or executing subsequent phases.

```bash
cd src
./IFJcompiler -s < ../samples/test.wren
```

---

## 🧪 Testing Infrastructure

The repository contains extensive automated testing suites. From the repository root, you can trigger validations using provided shell scripts:

### Run All Tests

```bash
cd tests
./run_all_tests.sh
```

### Execute Specific Modules

```bash
cd tests
./run_lex_tests.sh   # Lexical Analyzer Tests
./run_stx_tests.sh   # Syntactic Analyzer Tests
./run_sem_tests.sh   # Semantic Analyzer Tests
./run_gen_tests.sh   # Code Generator Tests
```

---

## 📖 Documentation

Detailed project architecture is maintained in `dokumentace.pdf` (in Czech).

To generate the API-level HTML documentation:

```bash
cd src
doxygen Doxyfile
```

The output can be accessed at `doc/html/index.html`.

---

## 📊 Evaluation Results

The compiler achieved an excellent foundational score. Points are measured in **mb** (the official grading metric benchmark).

### Core Compiler

| Implementation Category | Success | Points | Bad Return Code Loss | Output Mismatch Loss |
|------------------------|---------|--------|---------------------|---------------------|
| Lexical Analysis (Errors) | 95% | 172 / 180 mb | 4% | — |
| Syntax Analysis (Errors) | 98% | 247 / 250 mb | 1% | — |
| Semantic Analysis (Errors) | 84% | 178 / 210 mb | 15% | — |
| Semantic / Runtime Errors | 58% | 93 / 160 mb | 41% | — |
| Interpretation (Basic) | 89% | 220 / 247 mb | 0% | 10% |
| Interpretation (Expr. & Built-ins) | 100% | 314 / 314 mb | 0% | 0% |
| Interpretation (Complex Logic) | 74% | 352 / 471 mb | 25% | 0% |

### Language Extensions

| Extension Token | Success | Points Earned | Code Loss | Mismatch Loss |
|----------------|---------|--------------|-----------|---------------|
| BOOLTHEN | 78% | 78 / 100 mb | 7% | 15% |
| FUNEXP | 60% | 91 / 150 mb | 39% | 0% |
| OPERATORS | 40% | 10 / 25 mb | 60% | 0% |
| STATICAN | 13% | 30 / 215 mb | 86% | 0% |
| CYCLES | 12% | 12 / 100 mb | 88% | 0% |
| EXTSTAT | 0% | 0 / 25 mb | 100% | 0% |
| EXTFUN | 0% | 0 / 10 mb | 100% | 0% |
| ONELINEBLOCK | 0% | 0 / 25 mb | 100% | 0% |

### 🏆 Total Score (Without Extensions): **86% (1576 / 1832 mb)**
