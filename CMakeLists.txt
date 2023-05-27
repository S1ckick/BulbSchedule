cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(BULB_SCHEDULE VERSION 1.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_subdirectory(third-party)
add_subdirectory(src)
add_subdirectory(test)