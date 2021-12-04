# Copyright © Alexander Kaluzhnyy

add_compile_options(
    -Wall
    -Wextra
    -Werror
    -Wformat
    -Wformat-overflow
    -Wnull-dereference
    -Wmisleading-indentation
    -Wmissing-include-dirs
    -Wmultistatement-macros
    -Wmissing-braces
    -Wparentheses
    -Wno-return-local-addr
    -Wreturn-type
    -Wswitch
    -Wswitch-default
    -Wunused-but-set-parameter
    -Wunused-const-variable
    -Wuninitialized
    -Wunknown-pragmas
    -Wstrict-overflow
    -Wstring-compare
    -Wstringop-overflow
    -Walloca
    -Warith-conversion
    -Wbool-compare
    -Wduplicated-branches
    -Wduplicated-cond
    -Wzero-length-bounds
    -Wtautological-compare
    -Wtrampolines
    -Wtype-limits
    -Wundef
    -Wunused-macros
    -Wcast-function-type
    -Wwrite-strings
    -Wclobbered
    -Wconversion
    -Wdangling-else
    -Wdate-time
    -Wsizeof-pointer-memaccess
    -Wlogical-op
    -Wlogical-not-parentheses
    -Wmissing-declarations
    -Wmissing-field-initializers
    -Wpacked
    -Wpacked-not-aligned
    -Wrestrict
    -Wno-int-to-pointer-cast
    -Winvalid-pch
    -Wvector-operation-performance
    -Wvolatile-register-var
    -Wdisabled-optimization
    -Wstack-protector
    -Woverlength-strings
    -O2
    -ggdb3
    -pipe
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
    ${COMMON_COMPILER_FLAGS} \
    -Wbad-function-cast \
    -Winline \
    -Wstrict-prototypes \
    -Wmissing-parameter-type \
    -Wmissing-prototypes \
    -Wunsuffixed-float-constants \
")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    ${COMMON_COMPILER_FLAGS} \
    -std=c++20")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wall -Werror -ggdb3")

configure_file(CTestCustom.cmake CTestCustom.cmake COPYONLY)

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

add_compile_options(--coverage)
add_link_options(--coverage)

set(GCOVR_FLAGS
    -j
    --keep
    --root ${CMAKE_CURRENT_SOURCE_DIR}
    --html --html-details
    --exclude '\.\*/test/\.\*'
    --exclude-unreachable-branches
    --delete
)
add_custom_target(collect_coverage
    COMMAND echo "-- Coverage for C/C++ cleaning..."
    COMMAND rm -rf ${CMAKE_CURRENT_BINARY_DIR}/coverage/
    COMMAND find -name *.gcda | xargs rm -f
    COMMAND echo "-- Coverage for C/C++ running tests..."
    COMMAND bash -c 'ctest -j$(nproc)'
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/coverage/
    COMMAND echo "-- Coverage for C/C++ collecting..."
    COMMAND gcovr ${GCOVR_FLAGS} -o coverage/index.html
    COMMAND echo "-- Coverage for C/C++ report: ${CMAKE_CURRENT_BINARY_DIR}/coverage/index.html"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

