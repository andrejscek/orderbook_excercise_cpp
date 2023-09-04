cmake_minimum_required(VERSION 3.5)
project(DemoClient C CXX)

find_package(Boost REQUIRED COMPONENTS program_options)

set(SOURCE_FILES
    DemoClient.cpp
)

add_executable(DemoClient ${SOURCE_FILES})  ## STATIC or SHARED

target_include_directories(DemoClient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(DemoClient InpStrm Boost::program_options)

