cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(WiFiMicrophoneReceiver VERSION 0.2.0)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find necessary libraries
find_package(JACK) 
find_package(Threads)

# add include directorys
include_directories(src/include src/lib/rpi_nrf24/include src/lib/libopus/include ${JACK_INCLUDE_DIR})

# add source directorys
file(GLOB_RECURSE SOURCES src/cpp/*.cpp) # project sources
file(GLOB_RECURSE RPI_NRF24_SOURCES src/lib/rpi_nrf24/src/*.c)

# add the executable
add_executable(WiFiMicrophoneReceiver ${SOURCES} ${RPI_NRF24_SOURCES} src/WiFiMicrophoneReceiver.cpp)

# link libraries
target_link_libraries(WiFiMicrophoneReceiver ${JACK_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${PROJECT_SOURCE_DIR}/src/lib/libopus/libopus.a)