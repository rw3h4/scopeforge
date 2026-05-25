cmake_minimum_required(VERSION 3.20)
project(scopeforge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Useful defaults
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(scopeforge
  src/main.cpp
)
target_compile_options(scopeforge PRIVATE -Wall -Wextra -Wpedantic)

