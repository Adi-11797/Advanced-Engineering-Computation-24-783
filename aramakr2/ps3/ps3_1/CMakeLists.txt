add_executable(ps3_1 MACOSX_BUNDLE ps3.cpp data.h map.cpp ports.cpp tiles.cpp)
target_link_libraries(ps3_1 fssimplewindow)
target_include_directories(ps3_1 PUBLIC .)