cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(WiFiMicrophoneReceiver VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add include directory 
include_directories(src/include)

# add source directory 
file(GLOB SOURCES "src/cpp/*.cpp")

# add the executable
add_executable(WiFiMicrophoneReceiver ${SOURCES})