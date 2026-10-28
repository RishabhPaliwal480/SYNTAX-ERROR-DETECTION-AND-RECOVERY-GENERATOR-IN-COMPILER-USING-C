# Syntax Error Detection and Recovery Generator in Compiler Using C

[![RTU Kota](https://img.shields.io/badge/RTU%20Kota-5CAI4--02%20%7C%205CAI4--22-0a1c1a?style=flat-square&logo=c&logoColor=5eead4)](http://rtu.ac.in)
[![Department](https://img.shields.io/badge/Department-CSE%20(Artificial%20Intelligence)-28645b?style=flat-square)](https://jecrcfoundation.com)
[![Institution](https://img.shields.io/badge/Institution-JECRC%2C%20Jaipur-1b453f?style=flat-square)](https://jecrcfoundation.com)
[![Standard](https://img.shields.io/badge/C%20Standard-C99%20%2F%20C11%20Compliant-3d857a?style=flat-square)](https://en.cppreference.com/w/c)

An educational compiler front-end developed in standard C that performs coordinate-aware lexical tokenization, LL(1) predictive recursive-descent parsing, and non-destructive syntax error recovery. The project features GCC-style caret diagnostics (`^~~~`), an auto-repaired token stream generator, concrete syntax tree (CST) visualization, and an interactive modern visualizer workbench.

---

## 📋 Academic Details

- **Degree**: Bachelor of Technology (B.Tech)
- **Affiliation**: Rajasthan Technical University (RTU), Kota
- **Course Codes**:
  - `5CAI4-02`: Compiler Design (Theory)
  - `5CAI4-22`: Compiler Design Lab (Practical)
- **Department**: Computer Science & Engineering (Artificial Intelligence)
- **Institution**: Jaipur Engineering College and Research Centre (JECRC), Jaipur
- **Academic Session**: 2026–2027 (Semester V)
- **Project Supervisor**: Mrs. Renu Sharma (Assistant Professor)

### 👥 Team Members

| Name | Roll Number / University ID | Department |
| :--- | :--- | :--- |
| **Rishabh Paliwal** | `24EJCCA178` | CSE (AI), JECRC |
| **Shashwat Jain** | `24EJCCA190` | CSE (AI), JECRC |
| **Shagun Mishra** | `24EJCCA188` | CSE (AI), JECRC |

---

## 🎯 Problem Statement & Objective

Traditional entry-level parsers halt abruptly when encountering the first syntax error, producing cryptic messages without spatial context. This project addresses that challenge by:

1. Translating a restricted C-like arithmetic expression grammar into an **LL(1) left-recursion-eliminated specification**.
2. Tracking exact **line and column coordinates** for every token during lexical analysis.
3. Performing **phrase-level recovery** (virtual token insertions and stray token deletions) and **panic-mode synchronization** (at statement boundaries `;`).
4. Maintaining a **non-destructive recovery philosophy**: the raw source code is never altered in-place; instead, an auto-repaired equivalent stream is synthesized alongside clear diagnostic advice.

---

## 📐 Formal Grammar Specification

The language accepts arithmetic expression statements over identifiers, integer literals, binary operators (`+`, `-`, `*`, `/`), parentheses (`(` and `)`), and statement terminators (`;`).

### 1. Left-Recursion Elimination

The standard ambiguous/left-recursive arithmetic grammar:
$$E \rightarrow E + T \mid E - T \mid T$$
$$T \rightarrow T * F \mid T / F \mid F$$
$$F \rightarrow ( E ) \mid \mathbf{id} \mid \mathbf{num}$$

is converted to the predictive **LL(1)** form:

$$\begin{aligned}
S  &\rightarrow E \; \mathbf{;} \\
E  &\rightarrow T \; E' \\
E' &\rightarrow \mathbf{+} \; T \; E' \;\mid\; \mathbf{-} \; T \; E' \;\mid\; \varepsilon \\
T  &\rightarrow F \; T' \\
T' &\rightarrow \mathbf{*} \; F \; T' \;\mid\; \mathbf{/} \; F \; T' \;\mid\; \varepsilon \\
F  &\rightarrow \mathbf{(} \; E \; \mathbf{)} \;\mid\; \mathbf{id} \;\mid\; \mathbf{num}
\end{aligned}$$

### 2. Computed $\text{FIRST}$ and $\text{FOLLOW}$ Sets

| Non-Terminal ($A$) | $\text{FIRST}(A)$ | $\text{FOLLOW}(A)$ | Synchronizing Set ($\text{SYNC}$) |
| :--- | :--- | :--- | :--- |
| **$S$ (Statement)** | $\{ \mathbf{(}, \mathbf{id}, \mathbf{num} \}$ | $\{ \mathbf{\$}, \mathbf{(}, \mathbf{id}, \mathbf{num} \}$ | $\{ \mathbf{;}, \mathbf{\$} \}$ |
| **$E$ (Expression)** | $\{ \mathbf{(}, \mathbf{id}, \mathbf{num} \}$ | $\{ \mathbf{)}, \mathbf{;} \}$ | $\{ \mathbf{)}, \mathbf{;}, \mathbf{\$} \}$ |
| **$E'$ (Expr Tail)** | $\{ \mathbf{+}, \mathbf{-}, \varepsilon \}$ | $\{ \mathbf{)}, \mathbf{;} \}$ | $\{ \mathbf{)}, \mathbf{;}, \mathbf{\$} \}$ |
| **$T$ (Term)** | $\{ \mathbf{(}, \mathbf{id}, \mathbf{num} \}$ | $\{ \mathbf{+}, \mathbf{-}, \mathbf{)}, \mathbf{;} \}$ | $\{ \mathbf{+}, \mathbf{-}, \mathbf{)}, \mathbf{;}, \mathbf{\$} \}$ |
| **$T'$ (Term Tail)** | $\{ \mathbf{*}, \mathbf{/}, \varepsilon \}$ | $\{ \mathbf{+}, \mathbf{-}, \mathbf{)}, \mathbf{;} \}$ | $\{ \mathbf{+}, \mathbf{-}, \mathbf{)}, \mathbf{;}, \mathbf{\$} \}$ |
| **$F$ (Factor)** | $\{ \mathbf{(}, \mathbf{id}, \mathbf{num} \}$ | $\{ \mathbf{*}, \mathbf{/}, \mathbf{+}, \mathbf{-}, \mathbf{)}, \mathbf{;} \}$ | $\{ \mathbf{*}, \mathbf{/}, \mathbf{+}, \mathbf{-}, \mathbf{)}, \mathbf{;} \}$ |

---

## 🛠️ Recovery Strategies Implemented

```
                                Syntax Error Detected
                                          │
            ┌─────────────────────────────┴─────────────────────────────┐
            ▼                                                           ▼
    Phrase-Level Recovery                                      Panic-Mode Sync
   (Local In-Flight Fixes)                                  (Statement Boundaries)
  ┌─────────────────────────┐                             ┌─────────────────────────┐
  │ • Virtual <id> Insert   │                             │ • Unresolvable syntax   │
  │ • Virtual Delimiter (;) │                             │ • Scans forward to ';'  │
  │ • Stray ')' Deletion    │                             │ • Resets call stack for │
  │ • Redundant Op Deletion │                             │   next statement        │
  └─────────────────────────┘                             └─────────────────────────┘
```

1. **Phrase-Level Virtual Insertion**: When an operand is absent (e.g., `a + * b;`), the parser logs a diagnostic, synthesizes a virtual `<id>` into the repaired stream, and continues without throwing an unrecoverable exception.
2. **Phrase-Level Deletion**: Unmatched closing parentheses (e.g., `a + b ) * c;`) or unrecognized lexical characters (`@`, `$`) are cleanly discarded with an explicit deletion notice (`/*del:)*/`), allowing trailing terms to be validated.
3. **Panic-Mode Synchronization**: If an unparseable token cluster appears, the parser discards input up to the synchronizing token (`;` or `EOF`), successfully isolating the error to that specific statement.

---

## 📂 Repository File Structure

```
.
├── syntax_recovery_compiler.c     # Standalone C program (Lexer + Parser + CST)
├── index.html                     # Visualizer Workbench (Foresee-inspired UI)
└── README.md                      # Project documentation and viva guide
```

---

## ⚡ Compilation & Execution Guide

### 1. Running the Standalone C Compiler

The C implementation uses only standard ISO C libraries (`stdio.h`, `stdlib.h`, `string.h`, `ctype.h`) with zero external dependencies.

#### Linux / macOS / WSL:
```bash
# Compile with strict warnings
gcc -Wall -Wextra -std=c99 syntax_recovery_compiler.c -o syntax_recovery_compiler

# Execute the compiler
./syntax_recovery_compiler
```

#### Windows (MinGW / GCC / PowerShell):
```powershell
gcc -Wall -Wextra syntax_recovery_compiler.c -o syntax_recovery_compiler.exe
.\syntax_recovery_compiler.exe
```

### 2. Launching the Interactive Workbench

Open `index.html` in any modern web browser (Chrome, Firefox, Safari, Edge):
```bash
# On Linux
xdg-open index.html

# On macOS
open index.html

# Or simply double-click the index.html file in your file explorer
```

---

## 🧪 Benchmark Test Cases

The system includes 5 automated test cases directly reflecting the RTU project synopsis:

| Case | Test Input | Fault Identified | Recovery Strategy | Repaired Stream Output |
| :---: | :--- | :--- | :--- | :--- |
| **1** | `a + b * (c - 2);` | *None (Valid)* | Pass-through verification | `a + b * (c - 2);` |
| **2** | `a + * b;` | Missing right operand after `+` | Phrase-level virtual insertion | `a + <id> * b;` |
| **3** | `result + (alpha * 10 - beta;` | Missing closing parenthesis `)` | Phrase-level insertion | `result + (alpha * 10 - beta /*ins*/);` |
| **4** | `x + y * z` | Missing terminator `;` | Phrase-level boundary insertion | `x + y * z /*ins*/;` |
| **5** | `a + b ) * c;`<br>`val1 @ + 50;`<br>`p + q - ;` | Stray `)`, invalid `@`, missing right operand after `-` | Deletion + Lexical skip + Virtual `<id>` insertion | Multi-statement clean recovery |

---

## 🎓 Viva Voce Reference Questions & Answers

### Q1: Why did you eliminate left recursion from the grammar?
> **Answer**: Top-down predictive parsers (like recursive-descent) will enter an infinite loop when given a left-recursive grammar ($A \rightarrow A\alpha$), because the function for $A$ calls itself indefinitely before consuming any input tokens. Eliminating left recursion transforms the grammar into an LL(1) form suitable for recursive descent.

### Q2: What is the difference between Phrase-Level Recovery and Panic-Mode Recovery?
> **Answer**:
> - **Phrase-Level Recovery** makes localized corrections directly on the remaining token stream (such as inserting a missing semicolon or deleting an extraneous comma) so that the current production can complete.
> - **Panic-Mode Recovery** is simpler and more aggressive: when an error is detected, the parser skips all incoming tokens until a designated synchronizing token (such as `;` or `}`) from the non-terminal's $\text{FOLLOW}$ set is encountered.

### Q3: Why is non-destructive recovery preferred in an educational compiler?
> **Answer**: Modifying the original source string in-place can introduce cascading synthetic errors or hide the student's true syntax mistake. By keeping the input string read-only and emitting separate repair suggestions with caret indicators, the compiler clearly communicates both **what failed** and **how to fix it**.

### Q4: How is the lookahead token checked in your C code?
> **Answer**: Through the helper function `lookahead()`, which inspects `tokenStream[currentTokenIdx]` without advancing the index pointer. The parser advances only when `consumeToken()` is explicitly invoked after a grammar match.

---

## 📚 References

1. **Rajasthan Technical University, Kota**: B.Tech CSE (AI) Syllabus, Course `5CAI4-02`: *Compiler Design*.
2. **Rajasthan Technical University, Kota**: B.Tech CSE (AI) Syllabus, Course `5CAI4-22`: *Compiler Design Lab*.
3. Alfred V. Aho, Monica S. Lam, Ravi Sethi, Jeffrey D. Ullman, *Compilers: Principles, Techniques, and Tools* (2nd Edition), Pearson (The "Dragon Book").
4. Kenneth C. Louden, *Compiler Construction: Principles and Practice*, Cengage Learning.
5. John R. Levine, Tony Mason, Doug Brown, *lex & yacc*, O'Reilly Media.
