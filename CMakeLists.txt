cmake_minimum_required(VERSION 3.14)
project(async-runtime)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-g)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")

find_package(Threads REQUIRED)

file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM LIB_SOURCES ${CMAKE_SOURCE_DIR}/src/main.cpp)

add_library(async_runtime ${LIB_SOURCES})
target_include_directories(async_runtime PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(async_runtime PUBLIC Threads::Threads)

add_executable(main ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(main PRIVATE async_runtime)

include(CTest)
enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/tests/*.cpp)

foreach(TEST_SRC ${TEST_SOURCES})
        get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)

        add_executable(${TEST_NAME} ${TEST_SRC})
        target_link_libraries(${TEST_NAME} PRIVATE async_runtime GTest::gtest_main)
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
