cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(BulbSchedule VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(RESOURCE_FILES DATA_Files/*)

add_executable(scheduler src/algos.cpp src/greedy.cpp src/parser.cpp
  src/writer.cpp src/main.cpp src/validation.cpp)

target_include_directories(scheduler PUBLIC src third-party)

