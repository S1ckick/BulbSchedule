project(parser)

add_library(parser SHARED parser.cpp)
target_link_libraries(parser PRIVATE core date)
target_include_directories(parser PUBLIC ./)