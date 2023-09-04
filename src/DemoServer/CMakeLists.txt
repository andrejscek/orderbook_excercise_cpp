cmake_minimum_required(VERSION 3.5)
project(DemoServer C CXX)

find_package(Boost REQUIRED COMPONENTS program_options)

set(SOURCE_FILES
    DemoServer.cpp
)

add_executable(DemoServer ${SOURCE_FILES})  ## STATIC or SHARED

target_include_directories(DemoServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(DemoServer protomessages ExgBackE Boost::program_options)

