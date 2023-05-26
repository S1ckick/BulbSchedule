project(algos)


add_library(algos SHARED algos.cpp greedy.cpp)
target_link_libraries(algos PRIVATE core date)
target_include_directories(algos PUBLIC ./)