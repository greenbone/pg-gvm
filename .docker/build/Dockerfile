# Dockerfile for pg-gvm-$VERSION-$COMPILER-build

# Define ARG we use through the build
ARG VERSION=master
ARG BUILD_TYPE=Debug
ARG COMPILER=gcc

FROM greenbone/pg-gvm-$VERSION-$COMPILER-testing

# This will make apt-get install without question
ARG DEBIAN_FRONTEND=noninteractive

COPY . /usr/local/src/pg-gvm
WORKDIR /usr/local/src

# clone and install pg-gvm
RUN cd pg-gvm && \
   mkdir build && \
   cd build && \
   cmake -DCMAKE_BUILD_TYPE=Release .. && \
   make install
