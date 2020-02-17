cmake_minimum_required(VERSION 2.8)
project( michielsarra_p23rsc-tp )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( michielsarra_p23rsc-tp michielsarra_p23rsc-tp.cpp )
target_link_libraries( michielsarra_p23rsc-tp ${OpenCV_LIBS} )

