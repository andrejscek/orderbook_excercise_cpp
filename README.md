## info

This is a solution for a take-home test for the position of Senior Quantitative Developer. A 24-hour period was provided to complete the take-home challenge. It is a straightforward client-server application that utilizes the UDP protocol to exchange protobuf messages between the client and server, simulating a client interacting with an exchange. The exchange includes a functional matching engine, order tracking, and message output. Both the client and server are implemented in C++ and utilize the Boost library. The build process for both the client and server relies on CMake. Additionally, a Dockerfile is included to facilitate building and running the application within a Docker container.

the files provided for the challange are in the files_provided directory.

## note regarding scenario 13

To me, it makes more sense that the output, when an order matches two existing orders, creates two separate trade messages, one for each match, and a single best book change message at the end of matching. This way, the information regarding orderIds is retained. This is how I implemented it, so the output for scenario 13 is changed in the test cases.

## setup Docker

docker build -t takehome-test .

# run Docker

## run the image adn execute the tests

docker run -ti takehome-test

## run the image and get a shell

docker run -ti takehome-test /bin/bash

## run tests with ctest in shell

( cd build/test && ctest )

## run demo client example

./build/src/DemoClient/DemoClient

## run demo server example

./build/src/DemoServer/DemoServer
