# Define ARG we use through the build
ARG VERSION=stable

# We want gvmd to be ready so we use the docker image of gvmd
FROM greenbone/gvm-libs:${VERSION}

# This will make apt-get install without question
ARG DEBIAN_FRONTEND=noninteractive

WORKDIR /source

# Install postgresql
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    pkg-config \
    cmake \
    libglib2.0-dev \
    postgresql-server-dev-13 \
    pkg-config \
    libical-dev \
    pgtap && \
    rm -rf /var/lib/apt/lists/*

# Set up database
USER postgres
RUN /etc/init.d/postgresql start && \
    createuser -DRS root && \
    createdb -O root gvmd && \
    psql -d gvmd -c 'create role dba with superuser noinherit; grant dba to root;' && \
    psql -d gvmd -c 'create extension "uuid-ossp"; create extension "pgcrypto"; create extension "pgtap";' && \
    /etc/init.d/postgresql stop

# change back the user to root
USER root
