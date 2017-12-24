set(SANITIZERS "-fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic ${SANITIZERS}")
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CATCH_DIR ${CMAKE_CURRENT_BINARY_DIR}/catch)
if(NOT EXISTS ${CATCH_DIR}/catch.hpp)
    if (NOT EXISTS ${CATCH_DIR})
        make_directory(${CATCH_DIR})
    endif()
    file(
            DOWNLOAD
            https://raw.githubusercontent.com/catchorg/Catch2/v1.11.0/single_include/catch.hpp  ${CATCH_DIR}/catch.hpp
            STATUS
            status
            LOG
            log
    )
    list(GET status 0 status_code)
    list(GET status 1 status_string)

    if(NOT status_code EQUAL 0)
        message(FATAL_ERROR "error downloading catch: ${status_string}"
                "${log}")
    endif()
endif()

add_executable(self_test tests.cpp main.cpp ../include/lift.hpp)
target_link_libraries(self_test lift)
target_include_directories(self_test PRIVATE ${CATCH_DIR})
