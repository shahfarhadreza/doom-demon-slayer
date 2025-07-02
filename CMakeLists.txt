#
# CMakeLists.txt
# Created by Shah Farhad Reza on May 17, 2024
#
cmake_minimum_required( VERSION 3.28 )
project( doom VERSION 0.0.1 )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin) 

# OpenGL and GLFW
find_package( OpenGL REQUIRED )

set( SRCS 
    src/doom.c
    src/doom.h 
    )

add_executable( doom ${SRCS} )

if (WIN32)
target_compile_definitions(doom PUBLIC WIN32)
endif()

add_subdirectory(vendors/glfw)

target_include_directories(papercode PUBLIC src)
target_include_directories(papercode PUBLIC vendors)

target_link_libraries( doom ${OPENGL_LIBRARIES} )
target_link_libraries( doom glfw )





