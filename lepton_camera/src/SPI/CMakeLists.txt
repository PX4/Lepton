

# Include directory for header file
find_path(SPI_INCLUDE SPI.h ./)
include_directories(${SPI_INCLUDE})

#Add SPI library
add_library (SPI SPI.cpp)
target_include_directories (SPI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
