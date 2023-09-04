cmake_minimum_required(VERSION 3.5)
project(ke VERSION 0.0.1
  DESCRIPTION "My Kraken test engine"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")  #  -O3

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

option(BUILD_TESTING "Build tests" ON)

### set install directories
set(MAIN1_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin) ## for executables
set(ExgBackE_INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib) ## for libraries
set(ExgBackE_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include) ## for h files only
set(InpStrm_INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib) ## for libraries
set(InpStrm_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include) ## for h files only

# create ALL_HEADERS_DIR variable and for each folder in src a variable with name of the folder_HEADERS_DIR
file(GLOB_RECURSE src_dirs LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/src/*)
list(FILTER src_dirs INCLUDE REGEX "src/.+")
set(ALL_HEADERS_DIR)
foreach(dir ${src_dirs})
  list(APPEND ALL_HEADERS_DIR ${dir}) 
endforeach()
foreach(dir ${src_dirs})
  get_filename_component(dir_name ${dir} NAME)
  set(${dir_name}_HEADERS_DIR ${dir})
endforeach()
######################

# include(CTest)
add_subdirectory(src)
add_subdirectory(test)