# Copyright © Alexander Kaluzhnyy
project(json)
cmake_minimum_required(VERSION 3.21)

include(common.cmake)

add_library(json_obj OBJECT
    src/printer.c
    src/json.c
    src/log.c
)
target_include_directories(json_obj
    PUBLIC 
        include
)
set_target_properties(json_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(json SHARED $<TARGET_OBJECTS:json_obj>)
target_link_libraries(json PUBLIC json_obj)

# TESTING

enable_testing()

add_library(system_mock SHARED
    test/system_mock.cpp
    src/log.c
)


target_link_libraries(system_mock PUBLIC
    gmock
)

target_link_options(system_mock PUBLIC
    "-Wl,--wrap=malloc"
    "-Wl,--wrap=calloc"
    "-Wl,--wrap=realloc"
    "-Wl,--wrap=strdup"
    "-Wl,--wrap=free"
)

target_include_directories(system_mock PUBLIC test src)

add_executable(json_test
    test/json_nullptr_test.cpp
    test/json_init_from_value_test.cpp
    test/json_init_from_str_positive_test.cpp
    test/json_init_from_str_negative_test.cpp
    test/json_init_from_file_test.cpp
    test/json_graph_api_test.cpp
    test/json_graph_ref_test.cpp
    test/json_graph_system_test.cpp
    $<TARGET_OBJECTS:json_obj>
)

target_link_libraries(json_test PUBLIC
    json
    gtest
    gtest_main
    system_mock
)
gtest_discover_tests(json_test)

set(VALGRIND_PREFIX "valgrind \
        --error-exitcode=123 \
        --track-origins=yes \
        --trace-children=yes \
        --leak-check=full \
        --show-leak-kinds=all \
        --num-callers=100 \
        --trace-children=yes \
        --log-fd=1 ")

target_include_directories(json_test PUBLIC src include test)

if(DIVIDE_VALGRIND_TESTS EQUAL 1)
    file(WRITE  ${CMAKE_CURRENT_BINARY_DIR}/fakse.c "int main(void){return 0;}")
    file(WRITE  ${CMAKE_CURRENT_BINARY_DIR}/json_test_list_with_valgrind.sh "#! /bin/bash\n")
    file(APPEND  ${CMAKE_CURRENT_BINARY_DIR}/json_test_list_with_valgrind.sh "set -e\n")
    file(APPEND  ${CMAKE_CURRENT_BINARY_DIR}/json_test_list_with_valgrind.sh ${VALGRIND_PREFIX} ${CMAKE_CURRENT_BINARY_DIR}/json_test)
    file(APPEND  ${CMAKE_CURRENT_BINARY_DIR}/json_test_list_with_valgrind.sh " \"$@\"")
    file(
        CHMOD ${CMAKE_CURRENT_BINARY_DIR}/json_test_list_with_valgrind.sh 
        PERMISSIONS
            OWNER_READ
            OWNER_EXECUTE
            GROUP_READ
            GROUP_EXECUTE
    )
    add_executable(json_test_list_with_valgrind ${CMAKE_CURRENT_BINARY_DIR}/fakse.c)
    add_custom_command(
        TARGET json_test_list_with_valgrind
        POST_BUILD
        COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/json_test_list_with_valgrind.sh ${CMAKE_CURRENT_BINARY_DIR}/json_test_list_with_valgrind
    )
    add_dependencies(json_test_list_with_valgrind json_test)
    gtest_discover_tests(
        json_test_list_with_valgrind
        TEST_PREFIX valgrind_
    )
else()
add_test(
    NAME json_test_valgrind
    COMMAND valgrind
        --error-exitcode=123
        --track-origins=yes
        --trace-children=yes
        --leak-check=full
        --show-leak-kinds=all
        --num-callers=100
        --trace-children=yes
        --log-fd=1
         $<TARGET_FILE:json_test>
)
endif()

