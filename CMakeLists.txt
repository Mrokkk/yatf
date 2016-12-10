project(yatf CXX)
cmake_minimum_required(VERSION 3.0)

if(NOT CPP_STD)
    set(CPP_STD c++1z)
endif()
message(STATUS "[CPP_STD] C++ standard: ${CPP_STD}")

option(COVERAGE "Generate coverage reports" OFF)
message(STATUS "[COVERAGE] Generate coverage reports: ${COVERAGE}")

if(COVERAGE)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(COVERAGE_FLAGS "-O0 -fprofile-instr-generate -fcoverage-mapping")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
    else()
        set(COVERAGE_FLAGS "-O0 -fprofile-arcs -ftest-coverage")
    endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=${CPP_STD} -Wall -Wextra -Werror ${COVERAGE_FLAGS}")

add_executable(${PROJECT_NAME} tests/main.cpp tests/test_session.cpp tests/test_case.cpp tests/tests_list.cpp tests/printer.cpp)
target_link_libraries(${PROJECT_NAME} -lboost_unit_test_framework)

add_custom_target(tests-run COMMAND ./${PROJECT_NAME} DEPENDS ${PROJECT_NAME})

if(COVERAGE)
    add_custom_target(clean-coverage
        COMMAND find ${CMAKE_BINARY_DIR} -name '*.gcda' -exec rm {} "\;"
        DEPENDS tests
        COMMENT "Cleaning coverage data")
    add_dependencies(tests-run clean-coverage)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_custom_target(tests-cov
            DEPENDS ${PROJECT_NAME}
            COMMAND LLVM_PROFILE_FILE=tests.profdata ./${PROJECT_NAME}
            COMMAND llvm-profdata merge -instr tests.profdata -o merged.profdata
            COMMAND llvm-cov report ./${PROJECT_NAME} -instr-profile=merged.profdata
            COMMENT "Running LLVM coverage generating")
    else()
        add_custom_target(tests-cov
            COMMAND gcovr -r ${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}
            DEPENDS tests-run
            COMMENT "Running GCOVR coverage generating")
        add_custom_target(tests-cov-html
            COMMAND gcovr --html --html-details -o ut.coverage.html -r ${CMAKE_SOURCE_DIR}/include
            DEPENDS tests-run
            COMMENT "Running GCOVR coverage generating (HTML)")
    endif()
endif()

add_custom_target(tests-valgrind
    DEPENDS ${PROJECT_NAME}
    COMMAND valgrind --error-exitcode=1 ./${PROJECT_NAME}
    COMMENT "Running ${PROJECT_NAME} tests with valgrind")

