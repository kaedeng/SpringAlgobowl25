
cmake_minimum_required(VERSION 3.10)
project(Algobowl-Spring-25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fPIC -fopenmp -gdwarf-3 -O2 -march=native -mtune=native")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(OpenMP REQUIRED)

add_executable(main src/main/main.cpp src/main/input.cpp src/main/ttsolver.cpp src/main/board.cpp)

if(OpenMP_FOUND)
  target_link_libraries(main PUBLIC OpenMP::OpenMP_CXX)
  target_compile_options(main PUBLIC ${OpenMP_CXX_FLAGS})
endif()


enable_testing()

add_executable(
  run_tests
  src/main/board.cpp
  src/main/input.cpp
  src/main/ttsolver.cpp
  src/test/tests.cc
)

target_link_libraries(
  run_tests
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(run_tests)
