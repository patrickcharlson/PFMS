cmake_minimum_required(VERSION 4.2)
project(PFMS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if (MSVC)
    add_compile_options(/W4 /permissive-)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(PFMS
        src/main.cpp
        src/Account.cpp
        src/PFMS.cpp
        src/AuthService.cpp
        src/Sha256.cpp
        src/Bucket.cpp
        src/Color.cpp
        src/Transaction.cpp
)

# Test runner — separate executable, not built by default
add_executable(tests_runner EXCLUDE_FROM_ALL
        tests/tests.cpp
        src/Account.cpp
        src/AuthService.cpp
        src/Bucket.cpp
        src/Sha256.cpp
        src/Transaction.cpp
        tests/test_main.cpp
        tests/test_helpers.cpp
        tests/test_auth.cpp
        tests/test_buckets.cpp
        tests/test_helpers.h
        tests/test_safety.cpp
        tests/test_journal.cpp
        tests/test_deposit.cpp
        tests/test_transfer.cpp
        tests/test_withdraw.cpp
        tests/test_liquidity.cpp
        tests/test_crypto.cpp
)