project(cpp_common)

add_library(parser SHARED parser.cpp)
target_link_libraries(parser PUBLIC core date)
target_include_directories(parser PUBLIC ./)