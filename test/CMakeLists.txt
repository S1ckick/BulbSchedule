project(test)


set(RESOURCE_FILES DATA_Files/*)

add_executable(test main.cpp)
target_link_libraries(test PUBLIC parser)
set_target_properties(test PROPERTIES RESOURCE "${RESOURCE_FILES}")