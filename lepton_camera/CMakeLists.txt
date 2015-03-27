cmake_minimum_required(VERSION 2.8.3)
project(lepton_camera)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

find_package(catkin REQUIRED COMPONENTS
    roscpp nodelet
    message_generation
    )
find_package(Boost COMPONENTS program_options REQUIRED)




catkin_package(
    #  INCLUDE_DIRS include
    #  LIBRARIES bluefox2
    CATKIN_DEPENDS message_runtime
    #  DEPENDS system_lib
    )

include_directories(
    include
    ${catkin_INCLUDE_DIRS}
    )

add_subdirectory(src/video)
add_subdirectory(src/SPI)

# dependencies
add_dependencies(
    ${catkin_EXPORTED_TARGETS}
    ${PROJECT_NAME}_gencfg
    ${PROJECT_NAME}_gencpp
    ${${PROJECT_NAME}_EXPORTED_TARGETS}
    )

