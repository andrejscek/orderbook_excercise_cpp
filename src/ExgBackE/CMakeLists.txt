cmake_minimum_required(VERSION 3.5)
project(ExgBackE C CXX)

find_package(Boost REQUIRED COMPONENTS system)

set(SOURCE_FILES
    ExgBackE.hpp
    ob.hpp
    ExgBackE.cpp
    ob.cpp
)

add_library(ExgBackE ${SOURCE_FILES})

target_include_directories(ExgBackE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ExgBackE protomessages)


# install(TARGETS ExgBackE DESTINATION ${ExgBackE_INSTALL_LIB_DIR})
# install(FILES ExgBackE.hpp DESTINATION ${ExgBackE_INSTALL_INCLUDE_DIR})
