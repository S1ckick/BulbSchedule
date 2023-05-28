cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(BulbSchedule VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(RESOURCE_FILES DATA_Files/*)

set(CMAKE_PREFIX_PATH "third-party/or-tools-linux/lib/cmake") 
find_package(ortools REQUIRED CONFIG)

add_library(solver INTERFACE)
target_include_directories(solver INTERFACE ../third-party/or-tools-linux/include)
target_link_directories(solver INTERFACE ../third-party/or-tools-linux/lib)
target_link_libraries(solver INTERFACE ortools::ortools)

add_executable(scheduler src/algos.cpp src/greedy.cpp src/parser.cpp
  src/writer.cpp src/main.cpp src/validation.cpp src/bysolver.cpp)

target_include_directories(scheduler PUBLIC src third-party)
target_link_libraries(scheduler PRIVATE solver)

