add_library(hashutil yshash.cpp yshash.h)
target_include_directories(hashutil PUBLIC .)
target_link_libraries(hashutil simplebitmap)
