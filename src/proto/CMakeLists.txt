# Define your module and its source files
project(proto C CXX)

# Find the Protobuf package
find_package(Protobuf REQUIRED)

# Generate C++ source files from the .proto file
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS msgs.proto)

# Create a library from the generated source files
add_library(protomessages STATIC ${PROTO_SRCS} ${PROTO_HDRS})

# Include directories for this module
target_include_directories(protomessages PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(protomessages PUBLIC ${PROTOBUF_INCLUDE_DIRS})

# Link against the Protobuf library
target_link_libraries(protomessages ${PROTOBUF_LIBRARIES})