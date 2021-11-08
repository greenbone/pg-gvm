FROM greenbone/gvm-libs:main

# This will make apt-get install without question
ARG DEBIAN_FRONTEND=noninteractive

# Install Debian core dependencies required for building gvm with PostgreSQL
# support and not yet installed as dependencies of gvm-libs-core
RUN apt-get update && \
    apt-get install -y --no-install-recommends \ 
    build-essential \
    cmake \
    pkg-config \
    libglib2.0-dev \
    libgnutls28-dev \
    postgresql-server-dev-13 \
    pkg-config \
    libical-dev && \
    rm -rf /var/lib/apt/lists/*

COPY . /source
WORKDIR /source

# clone and install pg-gvm
RUN mkdir /build && \
    mkdir /install && \
    cd /build && \
    cmake -DCMAKE_BUILD_TYPE=Release /source && \
    make DESTDIR=/install install
