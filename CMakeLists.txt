cmake_minimum_required(VERSION 4.2)
project(PFMS)

set(CMAKE_CXX_STANDARD 17)

add_executable(PFMS
        src/main.cpp
        src/Account.cpp
        include/Account.h
        include/Bucket.h
        include/PFMS.h
        include/AuthService.h
        src/PFMS.cpp
        src/AuthService.cpp
        include/Sha256.h
        src/Sha256.cpp
#        tests/tests.cpp
)
