cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(BulbSchedule VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(third-party)
add_subdirectory(src)
add_subdirectory(test)
