cmake_minimum_required(VERSION 3.9)
project(lift)

add_library(lift INTERFACE)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(
        lift
        INTERFACE
        $<BUILD_INTERFACE:${INCLUDE_DIR}>
)

target_include_directories(
        lift
        INTERFACE
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)

set(MASTER_PROJECT OFF)
if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(MASTER_PROJECT ON)
endif()


if (MASTER_PROJECT)
    add_subdirectory(test)
endif()
