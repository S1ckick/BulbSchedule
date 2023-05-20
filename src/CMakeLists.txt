# include_directories(third-party)

add_library(core INTERFACE)
target_include_directories(core INTERFACE ./)

add_subdirectory(parser)

