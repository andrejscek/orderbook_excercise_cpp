FROM ubuntu:23.10

ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /app

RUN apt-get update && \
    apt-get install -y build-essential cmake libboost-dev libboost-system-dev libboost-program-options-dev python3-dev git protobuf-compiler libprotobuf-dev

COPY . .

RUN mkdir build && \
    cd build && \
    cmake .. && \
    make

CMD ( cd build/test && ctest )
