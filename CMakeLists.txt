cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(WiFiMicrophoneReceiver VERSION 0.2.0)

set(CMAKE_MODULE_PATH ${WiFiMicrophoneReceiver_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find necessary libraries
find_package(JACK) 
find_package(OPUS)
find_package(Threads)

# add include directorys
include_directories(src/include ${JACK_INCLUDE_DIR} ${OPUS_INCLUDE_DIR})

# add source directorys
file(GLOB_RECURSE SOURCES src/cpp/*.cpp) # project sources

# add the executable
add_executable(WiFiMicrophoneReceiver ${SOURCES} src/WiFiMicrophoneReceiver.cpp)
add_executable(WiFiMicrophoneTCP ${SOURCES} src/WiFiMicrophoneTCP.cpp)


# link libraries
target_link_libraries(WiFiMicrophoneReceiver ${JACK_LIBRARY} ${OPUS_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(WiFiMicrophoneTCP ${JACK_LIBRARY} ${OPUS_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})