project(test)


set(RESOURCE_FILES DATA_Files/*)

add_executable(test main.cpp validation.cpp)
target_link_libraries(test PRIVATE parser algos core date)