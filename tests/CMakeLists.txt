include(cmake-utils/utils.cmake)
include(cmake-utils/coverage.cmake)
include(cmake-utils/sanitizers.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=${CPP_STD} -Wall -Wextra -Werror -fno-exceptions -g3")

if(SANITIZERS)
    add_sanitizers_flags()
endif()

if(COVERAGE)
    add_coverage_flags()
endif()

add_executable(${PROJECT_NAME} main.cpp test_session.cpp test_case.cpp printer.cpp list.cpp mock.cpp)
target_link_libraries(${PROJECT_NAME} -lboost_unit_test_framework)

add_run_target(${PROJECT_NAME} tests-run)

if(COVERAGE)
    add_coverage_targets(${PROJECT_NAME} tests-run tests ${CMAKE_SOURCE_DIR}/include)
endif()

if(NOT SANITIZERS)
    add_valgrind_target(${PROJECT_NAME} tests-valgrind)
endif()

