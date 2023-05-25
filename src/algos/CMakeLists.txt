project(algos)


add_library(algos SHARED algos.cpp)
target_link_libraries(algos PUBLIC core date)
target_include_directories(algos PUBLIC ./)

add_subdirectory(greedy)