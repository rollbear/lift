cmake_minimum_required(VERSION 3.9)
project(higher_order_functions)

set(SANITIZERS "-fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic ${SANITIZERS}")
set(CMAKE_CXX_STANDARD 17)
include_directories(../catch/single_include)
add_executable(higher_order_functions main.cpp include/lift.hpp)
target_include_directories(higher_order_functions PRIVATE include)
