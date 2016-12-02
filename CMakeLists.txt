project(yatf CXX)
cmake_minimum_required(VERSION 3.0)

add_executable(${PROJECT_NAME} tests/main.cpp tests/self_tests.cpp)

add_custom_target(tests-run COMMAND ./${PROJECT_NAME} DEPENDS ${PROJECT_NAME})

