cmake_minimum_required(VERSION 3.5)

# Check if BUILD_TESTING is set, and if so, configure testing
if (BUILD_TESTING)
    # Add Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.11.0
    )
    FetchContent_MakeAvailable(googletest)

    enable_testing() # needed?
    include(GoogleTest)

    include_directories(${ALL_HEADERS_DIR})

    # # Define test sources manually
    # set(TEST_SOURCES
    #     test_integration.cpp
    #     test_unit.cpp
    # )

    # Automatically find all .cpp files
    file(GLOB_RECURSE test_files ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

    set(TEST_SOURCES)
    foreach(file ${test_files})
    if(${file} MATCHES ".*\\.cpp$") 
        list(APPEND TEST_SOURCES ${file})
    endif()
    endforeach()

    foreach(TEST_SOURCE ${TEST_SOURCES})
        # Extract the test name from the source file name
        get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
 
        # Add the test executable
        add_executable(${TEST_NAME} ${TEST_SOURCE})

        ## add all libraries here, defined in each folder's CMakeLists.txt
        target_link_libraries(${TEST_NAME} PRIVATE gtest_main ExgBackE InpStrm)  
        target_include_directories(${TEST_NAME} PRIVATE ${ALL_HEADERS_DIR})

        # Discover tests
        gtest_discover_tests(${TEST_NAME})
        # add_test(${TEST_NAME} ${TEST_SOURCE})
    endforeach()

endif()
