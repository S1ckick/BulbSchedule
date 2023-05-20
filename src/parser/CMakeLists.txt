project(cpp_common)

add_library(parser SHARED parser.cpp parser.h)
target_link_libraries(parser PUBLIC date)
target_include_directories(parser PUBLIC ./)