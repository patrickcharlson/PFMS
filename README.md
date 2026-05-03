# PFMS — Personal Finance & Liquidity Management System

A C++17 console application implementing the SRS by Patrick Charlson, Sonam Wangdi, and Sithmi Hirasha.

PFMS lets you partition a single account into named "buckets" (Rent, Food, Savings, etc.) and shows your real-time **Safe to Spend** balance — your total minus anything you've marked as committed.

## Requirements

- A C++17 compiler:
    - **macOS**: clang (comes with Xcode Command Line Tools — `xcode-select --install`)
    - **Linux**: g++ 9.0 or later (`sudo apt install g++` on Ubuntu)
    - **Windows**: MSVC 2019 or later, or g++ via MinGW
- **Optional**: CMake 3.15+ if you want to use the CMake build, or CLion which handles it for you

No third-party libraries are needed — only the C++ Standard Library.

## Getting the code

Extract `pfms.zip` to a folder of your choice. You should end up with a structure like:

```
pfms/
├── CMakeLists.txt
├── README.md
├── include/        (header files)
├── src/            (source files)
└── tests/          (test suite)
```

Open a terminal and `cd` into the `pfms/` folder.

## Running the program

### Option 1 — Command line (simplest)

```bash
g++ -std=c++17 -Iinclude src/*.cpp -o pfms
./pfms
```

On Windows with g++, the output is `pfms.exe` and you run it as `pfms.exe`.

### Option 2 — CMake

```bash
mkdir build && cd build
cmake ..
cmake --build .
./pfms
```


## Using the program

When the program starts, you'll see a Welcome screen with three options:

```
========================================
 PFMS — WELCOME  [? = Help]
========================================
 [1] Login
 [2] Register
 [3] Exit
```

**First time?** Pick `2` to register a new account. You'll be asked for a username and a password (minimum 4 characters). After registration you're logged in automatically and taken to the Main Menu.

**Already registered?** Pick `1` to log in. You get three attempts; after three failed logins the session locks and you'll need to restart the program.

**The Main Menu** lets you:

1. View your **Account Summary** — shows Safe to Spend, total balance, and all your buckets
2. **Manage Buckets** — create, edit, delete, or toggle the committed status on buckets
3. **Deposit** — add funds. They're auto-distributed across your buckets by their configured percentages
4. **Withdraw** — take funds out. Withdrawals exceeding your Safe to Spend trigger a warning before touching committed funds
5. **Transfer** funds from the unallocated pool into a specific bucket
6. View your **Transaction Journal** for the session
7. **Logout** — clears all session data

**Help is always available** — type `?` at any input prompt to see context-sensitive help.

## Try this short workflow

To see all the main features in action:

1. Register an account (e.g. username `patrick`, password `test1234`)
2. Manage Buckets → Create Bucket → name `Rent`, percentage `40`, mark as Committed (`Y`)
3. Manage Buckets → Create Bucket → name `Food`, percentage `15`, not committed (`N`)
4. Manage Buckets → Create Bucket → name `Savings`, percentage `20`, not committed (`N`)



## Troubleshooting

**"Cannot find file 'PFMS.h'"** — make sure you're running the compiler from inside the `pfms/` folder, and that the `-Iinclude` flag is included.

**Linker errors about missing symbols** — make sure you're compiling all the `.cpp` files in `src/` together. Using `src/*.cpp` (with the wildcard) picks them all up.

**Program prints garbled characters on Windows** — make sure your terminal is set to UTF-8 (`chcp 65001` in cmd.exe). The program uses an em dash (`—`) in headers.