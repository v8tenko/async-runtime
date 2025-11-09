cmake_minimum_required(VERSION 3.14)
project(async-runtime)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-g)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(main ${APP_SOURCES})

foreach(TEST_SRC ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    get_filename_component(SRC_DIR ${TEST_SRC} DIRECTORY)
    file(GLOB SOURCES "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.tpp")
    list(REMOVE_ITEM SOURCES ${TEST_SRC})

    add_executable(${TEST_NAME} ${TEST_SRC} ${SOURCES})
    target_link_libraries(${TEST_NAME} gtest_main)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
