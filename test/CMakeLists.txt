project(test)


set(RESOURCE_FILES DATA_Files/*)

add_executable(test main.cpp validation.cpp)
target_include_directories(test PUBLIC ./)
target_link_libraries(test PRIVATE parser algos core date)