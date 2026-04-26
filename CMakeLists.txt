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
        #        tests/tests.cpp
)
