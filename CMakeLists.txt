cmake_minimum_required(VERSION 3.16)
project(serial2screen)

find_package( OpenCV REQUIRED )
find_package( Boost REQUIRED COMPONENTS program_options )
find_package( Threads REQUIRED )

include_directories(${OpenCV_INCUDES})
include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 20)

add_executable(serial2screen main.cpp)
target_link_libraries( serial2screen ${OpenCV_LIBS} ${Boost_LIBRARIES} Threads::Threads)