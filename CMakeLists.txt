cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(WiFiMicrophoneReceiver VERSION 0.2.0)

set(CMAKE_MODULE_PATH ${WiFiMicrophoneReceiver_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# find JACK
find_package(JACK)

# fine OPUS
find_package(OPUS)

# add include directorys
include_directories(src/include ${JACK_INCLUDE_DIR} ${OPUS_INCLUDE_DIR})

# add source directorys
file(GLOB_RECURSE SOURCES src/cpp/*.cpp) # project sources

# add the executable
add_executable(WiFiMicrophoneReceiver ${SOURCES} src/WiFiMicrophoneReceiver.cpp)

# link JACK
target_link_libraries(WiFiMicrophoneReceiver ${JACK_LIBRARY} ${OPUS_LIBRARY})