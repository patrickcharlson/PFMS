# PFMS — Personal Finance & Liquidity Management System

A C++17 console application that helps users separate committed financial obligations from genuinely available funds.

Most banking apps show you a single number — your total balance — and let you guess what's actually safe to spend. PFMS solves that by partitioning a single account into **virtual buckets** (Rent, Food, Savings, etc.) and surfacing one trustworthy figure: **Safe to Spend** — your total balance minus anything you've marked as committed.

**Version 1.0** · **Patrick Charlson · Sonam Wangdi · Sithmi Hirasha**

---

## Table of contents

1. [Features](#features)
2. [Requirements](#requirements)
3. [Project layout](#project-layout)
4. [Building](#building)
5. [Running the application](#running-the-application)
6. [Running the tests](#running-the-tests)
7. [Architecture](#architecture)
8. [Business rules](#business-rules)
9. [Known limitations](#known-limitations)
10. [Troubleshooting](#troubleshooting)

---

## Features

| Feature | What it does | SRS |
|---|---|---|
| **User authentication** | Register, log in, log out. SHA-256 password hashing. Account lockout after 3 failed attempts. | §4.1 |
| **Virtual buckets** | Create, edit, delete, and toggle the *committed* status of named pools. Allocations sum to ≤ 100%. | §4.2 |
| **Smart-Distribute deposits** | Each deposit allocates funds proportionally across buckets per their configured percentage. Remainder lands in the unallocated pool. | §4.3 |
| **Liquidity calculator** | "Safe to Spend" = total − committed. Recalculated live on every change. | §4.4 |
| **Withdrawal & validation** | Three outcomes: allowed (within Safe to Spend), warned + Y/N confirm (exceeds Safe to Spend, within total), hard-rejected (exceeds total balance). | §4.5 |
| **Manual transfers** | Move money from the unallocated pool into a specific bucket. | §4.3 REQ-5 |
| **Transaction journal** | Chronological record of every successful deposit, withdrawal, and transfer with timestamps. | §4.3 REQ-4 |
| **ANSI color output** | `[ERROR]` red, `*** WARNING ***` yellow, `Safe to Spend` bold green. Auto-enabled on Windows 10+, macOS, and Linux. | §3.1 |

---

## Requirements

A C++17-capable compiler:

- **macOS** — clang (comes with Xcode Command Line Tools — install with `xcode-select --install`)
- **Linux** — `g++` 9.0 or newer (`sudo apt install g++` on Ubuntu/Debian)
- **Windows** — `g++` via MinGW or MSYS2, or MSVC 2019 or newer

**Optional:** CMake 3.15 or later (or CLion, which manages CMake automatically). The project also builds without CMake using the direct `g++` commands shown below.

**No third-party libraries.** Standard library only.

---

## Project layout

```
PFMS/
├── CMakeLists.txt              # Build configuration
├── README.md                   # This file
├── include/                    # Header files
│   ├── Account.h               # Domain: buckets, balance, journal, math
│   ├── AuthService.h           # Domain: register, login, lockout
│   ├── Bucket.h                # Domain: name, percentage, committed flag
│   ├── Color.h                 # ANSI color helpers
│   ├── PFMS.h                  # UI layer: console screens
│   ├── Sha256.h                # Domain: FIPS 180-4 password hashing
│   └── Transaction.h           # Domain: type, amount, timestamp
├── src/                        # Implementation files
│   ├── Account.cpp
│   ├── AuthService.cpp
│   ├── Bucket.cpp
│   ├── Color.cpp
│   ├── PFMS.cpp                # All console screens
│   ├── Sha256.cpp
│   ├── Transaction.cpp
│   └── main.cpp                # Entry point
└── tests/                      # Automated test suite
    ├── test_helpers.h          # Shared CHECK macro, color helpers
    ├── test_helpers.cpp        # Global counters, reconciledTotal()
    ├── test_main.cpp           # Test runner entry point
    ├── test_auth.cpp           # Authentication tests
    ├── test_buckets.cpp        # Bucket management tests
    ├── test_crypto.cpp         # SHA-256 tests
    ├── test_deposit.cpp        # Smart-Distribute tests
    ├── test_journal.cpp        # Transaction journal tests
    ├── test_liquidity.cpp      # Safe to Spend tests
    ├── test_safety.cpp         # Atomicity & invariant tests
    ├── test_transfer.cpp       # Manual transfer tests
    └── test_withdraw.cpp       # Withdrawal & BR-05 tests
```

---

## Building

There are two executables: the **application** (`pfms`) and the **test runner** (`pfms_tests`).

### Build with `g++` directly (simplest)

From the project root:

**Application:**
```bash
g++ -std=c++17 -Iinclude src/*.cpp -o pfms
```

**Tests:**
```bash
g++ -std=c++17 -Iinclude src/Bucket.cpp src/Transaction.cpp src/Account.cpp src/Sha256.cpp src/AuthService.cpp tests/test_*.cpp -o pfms_tests
```

The test build deliberately omits `src/main.cpp` and `src/PFMS.cpp` — tests exercise the domain layer directly without the UI.

On Windows, append `.exe` to the output names:

```bash
g++ -std=c++17 -Iinclude src/*.cpp -o pfms.exe
g++ -std=c++17 -Iinclude src/Bucket.cpp src/Transaction.cpp src/Account.cpp src/Sha256.cpp src/AuthService.cpp tests/test_*.cpp -o pfms_tests.exe
```

### Build with CMake

```bash
cmake -B build
cmake --build build                            # builds the PFMS application
cmake --build build --target tests_runner      # builds the test runner
```

The `tests_runner` target is marked `EXCLUDE_FROM_ALL`, so it isn't built by default — you have to ask for it explicitly. This keeps the main build fast.

### Build in CLion

Open the project folder. CLion auto-detects `CMakeLists.txt`. Two run targets appear in the dropdown:

- `PFMS` — the application
- `tests_runner` — the test suite

Pick whichever you want and hit Run.

---

## Running the application

```bash
./pfms                  # macOS / Linux
.\pfms.exe              # Windows (cmd.exe)
./pfms.exe              # Windows (Git Bash, MSYS2)
```

You'll see the welcome banner and a Welcome screen offering Login, Register, or Exit. Type `?` at any prompt for context-sensitive help.

### Quick walkthrough

1. **Register** — choose username `patrick`, password `test1234`. Auto-logs you into the Main Menu.
2. **Manage Buckets** — create `Rent` at 40% (mark as committed), `Food` at 15%, `Savings` at 20%.
3. **Deposit** — enter `1000`. Watch Smart-Distribute allocate the funds.
4. **Account Summary** — Rent $400, Food $150, Savings $200, Unallocated $250, Safe to Spend $600.
5. **Withdraw** — try `300` (proceeds), then `800` (warning + Y/N), then `9999` (hard reject).
6. **Journal** — see every successful operation logged chronologically.
7. **Logout** — returns to Welcome.

---

## Running the tests

```bash
./pfms_tests
```

Expected output ends with:

```
RESULTS:
  ✓ Passed:    194    (100.0%)
  ✓ Failed:      0
  ▸ Sections:   45
  ⏱  Duration:    47 ms

  ALL TESTS PASSED   Suite is healthy.
```

The test runner emits ANSI color: green PASS, red FAIL with reverse-video badges, cyan section headers, and a summary card. On Windows, color is enabled automatically via `Color::enable()`.

### What's covered

53 test functions across 9 feature areas, totalling roughly 194 individual checks:

| File | Area | Roughly covers |
|---|---|---|
| `test_auth.cpp` | §4.1 | Registration, login, lockout, session lifecycle, case-insensitive usernames |
| `test_buckets.cpp` | §4.2 | Create, edit, delete, toggle, BR-03 cap, duplicate names, whitespace |
| `test_deposit.cpp` | §4.3 | Smart-Distribute math, arithmetic integrity (QA-01), edge amounts |
| `test_transfer.cpp` | §4.3 REQ-5 | Manual transfer happy path and rejection cases |
| `test_withdraw.cpp` | §4.5 | All three `WithdrawCheck` outcomes, BR-05, drain priority |
| `test_liquidity.cpp` | §4.4 | Safe to Spend computation across state changes |
| `test_journal.cpp` | §4.3 REQ-4 | Successful operations logged, rejected ones not, chronological order |
| `test_crypto.cpp` | SEC-02 | SHA-256 against FIPS 180-4 reference vectors |
| `test_safety.cpp` | SR-04, QA-01 | Atomicity, no partial state, end-to-end scenarios |

The split structure means changing one feature only triggers a recompile of that one test file — about 3× faster iteration than a single monolithic `tests.cpp`.

---

## Architecture

PFMS is split into two layers, with a clear contract between them.

### UI layer (`PFMS` class)

Responsible for everything the user sees and types: console screens, menu loops, prompt formatting, error and warning display, color output, input parsing, Y/N confirmation dialogs. Knows nothing about how buckets work — it asks the domain layer to do the work and renders the result.

Files: `include/PFMS.h`, `src/PFMS.cpp`, `src/main.cpp`, `include/Color.h`, `src/Color.cpp`.

### Domain layer

Responsible for the business rules: bucket allocation math, BR-03 cap enforcement, BR-05 hard-reject, SR-02 warning logic, drain priority, password hashing, transaction logging. Knows nothing about consoles or input — it works with strings and numbers and returns `Status` or `LoginOutcome` results.

Files: `include/Account.h`, `src/Account.cpp`, `include/AuthService.h`, `src/AuthService.cpp`, `include/Bucket.h`, `src/Bucket.cpp`, `include/Transaction.h`, `src/Transaction.cpp`, `include/Sha256.h`, `src/Sha256.cpp`.

This separation is what makes the domain easily testable — `tests/test_*.cpp` calls `Account` and `AuthService` directly without simulating a terminal. It also means a future GUI or web frontend could replace the UI layer without rewriting the rules.

---

## Business rules

| ID | Rule |
|---|---|
| **BR-01** | Only one user account active at a time per session |
| **BR-03** | Total bucket allocation must never exceed 100% |
| **BR-04** | Withdrawals from committed funds require explicit Y/N confirmation |
| **BR-05** | Withdrawals exceeding total balance are hard-rejected — no override path |
| **SR-02** | `*** WARNING ***` prefix on every committed-funds withdrawal prompt |
| **SR-04** | Operations are atomic — failed operations leave no partial state |
| **SEC-02** | Passwords stored as SHA-256 hashes, never plain text |
| **SEC-03** | Lockout after 3 consecutive failed login attempts |
| **QA-01** | Sum of bucket balances + unallocated = total balance, exactly |

---

## Known limitations

These are deliberate scope boundaries set by the v1.0 SRS, not bugs:

- **No persistent storage.** All data lives in memory and is lost when the process exits. The SRS forbids file storage in this version. A future version might add JSON/XML serialisation.
- **Single account only.** One user can configure one set of buckets. Multi-account support is out of scope.
- **No undo.** Once a withdrawal is confirmed, it's logged. The journal records what happened but doesn't reverse it.
- **Console only.** No GUI, no web, no API. The UI layer is deliberately replaceable, but only the console version exists.
- **English ASCII only.** Usernames and bucket names are case-folded with `std::tolower`, which only handles ASCII. Non-ASCII characters work but aren't case-folded.
- **No password recovery.** Forgotten passwords mean a new account.

---

## Troubleshooting

### `error: 'uint32_t' has not been declared`

Add `#include <cstdint>` to the top of `include/Sha256.h`. macOS hides this through transitive includes; Linux and strict MSVC don't.

### `error: 'WithdrawCheck' has not been declared` (or `Account::WithdrawCheck`)

`WithdrawCheck` is at file scope in `Account.h`, not nested inside `Account`. Reference it as `WithdrawCheck::Ok`, not `Account::WithdrawCheck::Ok`.

### Tests fail with "case-insensitive duplicate" assertions

`AuthService::registerUser` and `AuthService::login` need to lowercase the username before doing the map lookup. Both functions must use the same canonical form. See `src/AuthService.cpp` for the `toLowerUsername` helper.

### Color escape codes show as literal `\033[...]` in output

The terminal isn't ANSI-capable, or `Color::enable()` wasn't called. The application calls it automatically at startup. The test runner does too. If you're still seeing escape codes, your terminal might be old (legacy `cmd.exe` pre-Windows 10) or you're capturing output to a file (use `cat output.txt` to see colors, or `sed 's/\x1b\[[0-9;]*m//g'` to strip them).

### "Auto-login failed; please log in manually" after registering

The session was locked from earlier failed login attempts before registration. Either restart the application, or apply the `resetLockState()` fix to clear the lock on successful registration. See `src/AuthService.cpp`.

### Build fails with "no such file or directory" on tests/test_*.cpp

Some shells don't expand the `*` glob. Either use `bash` (which does), or list the test files explicitly:

```bash
g++ -std=c++17 -Iinclude src/Bucket.cpp src/Transaction.cpp src/Account.cpp src/Sha256.cpp src/AuthService.cpp tests/test_helpers.cpp tests/test_main.cpp tests/test_auth.cpp tests/test_buckets.cpp tests/test_crypto.cpp tests/test_deposit.cpp tests/test_journal.cpp tests/test_liquidity.cpp tests/test_safety.cpp tests/test_transfer.cpp tests/test_withdraw.cpp -o pfms_tests
```

### CLion doesn't show the `tests_runner` target

It's marked `EXCLUDE_FROM_ALL` in `CMakeLists.txt`. Reload the CMake project (File → Reload CMake Project), then look in the run-target dropdown. If still missing, your CMake build may have failed silently — check the CMake panel for errors.

---

## License & attribution

Coursework submission for **SEP401** at **Torrens University Australia**. Not licensed for commercial use.

SHA-256 implementation in `src/Sha256.cpp` is a hand-rolled C++ port of the FIPS 180-4 specification. Tested against the official NIST reference vectors — see `tests/test_crypto.cpp`.