project(greedy)

add_library(greedy SHARED greedy.cpp)
target_link_libraries(greedy PRIVATE core date)
target_include_directories(greedy PUBLIC ./)