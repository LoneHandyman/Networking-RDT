cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
  message( FATAL_ERROR "This project can run only in Linux or Unix." )
endif()

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

project(RDT)

include_directories(include)

file(GLOB SOURCE_FILES
  "src/Algorithm/*cpp"
  "src/RDT/*cpp"
  "src/UDPWrapper/*cpp"
)

add_executable(${PROJECT_NAME} "src/test.cpp" ${SOURCE_FILES})