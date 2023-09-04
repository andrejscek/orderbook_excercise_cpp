cmake_minimum_required(VERSION 3.5)
project(InpStrm C CXX)

find_package(Boost REQUIRED COMPONENTS system)

set(SOURCE_FILES
    InpStrm.hpp
    InpStrm.cpp
)

add_library(InpStrm ${SOURCE_FILES})  ## STATIC or SHARED


target_include_directories(ExgBackE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(InpStrm protomessages ExgBackE)

# install(TARGETS InpStrm DESTINATION ${InpStrm_INSTALL_LIB_DIR})
# install(FILES InpStrm.hpp DESTINATION ${InpStrm_INSTALL_INCLUDE_DIR})
