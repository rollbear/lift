set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CATCH2_PREFIX)
    set(CATCH_DIR "${CATCH2_PREFIX}/include")
else ()
    set(CATCH_DIR ${CMAKE_CURRENT_BINARY_DIR}/catch)
    set(CATCH_MAIN main.cpp)
    if(NOT EXISTS ${CATCH_DIR}/catch.hpp)
        if (NOT EXISTS ${CATCH_DIR})
            make_directory(${CATCH_DIR}/catch2)
        endif()
        file(
                DOWNLOAD
                https://github.com/catchorg/Catch2/releases/download/v2.13.9/catch.hpp  ${CATCH_DIR}/catch2/catch.hpp
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
endif()
add_executable(self_test tests.cpp ${CATCH_MAIN} ../include/lift.hpp)
target_link_libraries(self_test lift)
target_include_directories(self_test PRIVATE ${CATCH_DIR})
if (CATCH2_PREFIX)
    target_compile_definitions(self_test PRIVATE CATCH2_PREBUILT)
    target_link_directories(self_test PRIVATE "${CATCH2_PREFIX}/lib")
    target_link_libraries(self_test Catch2Main Catch2)
endif()
