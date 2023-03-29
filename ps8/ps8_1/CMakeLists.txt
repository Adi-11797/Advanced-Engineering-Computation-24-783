add_library(mesh mesh.h mesh.cpp)
target_link_libraries(mesh ysclass)


add_library(glutil glutil.h glutil.cpp)
target_link_libraries(glutil ysclass)


add_executable(ps8_1 MACOSX_BUNDLE ps8_1.cpp glutil.h glutil.cpp mesh.h mesh.cpp lattice.h)
target_link_libraries(ps8_1 fssimplewindow ysclass mesh glutil)