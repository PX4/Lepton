# Add executable leptonvideo
add_executable (leptonvideo lepton.cpp )
target_link_libraries (leptonvideo SPI ${catkin_LIBRARIES})
