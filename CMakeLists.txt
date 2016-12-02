project(yatf CXX)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -Wextra -Werror")

add_executable(${PROJECT_NAME} tests/main.cpp tests/self_tests.cpp)

add_custom_target(tests-run COMMAND ./${PROJECT_NAME} DEPENDS ${PROJECT_NAME})

