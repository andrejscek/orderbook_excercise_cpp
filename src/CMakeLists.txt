cmake_minimum_required(VERSION 3.5)
# project(main1)

add_subdirectory(ExgBackE)
add_subdirectory(InpStrm)
add_subdirectory(proto)
add_subdirectory(DemoClient)
add_subdirectory(DemoServer)

# set(SOURCE_FILES run_ex.cpp)

# add_executable(main1 ${SOURCE_FILES})
# target_link_libraries(main1 ExgBackE InpStrm)
# install(TARGETS main1 DESTINATION ${MAIN1_INSTALL_BIN_DIR})


