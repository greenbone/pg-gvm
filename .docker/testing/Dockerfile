# Dockerfile for pg-gvm-$VERSION-$COMPILER-testing

# Define ARG we use through the build
ARG VERSION=master
ARG BUILD_TYPE=Debug
ARG COMPILER=gcc

# We want gvmd to be ready so we use the build docker image of gvmd
FROM greenbone/gvmd-$VERSION-$COMPILER-build

# This will make apt-get install without question
ARG DEBIAN_FRONTEND=noninteractive

# Install postgresql
RUN apt-get update && apt-get install --assume-yes \
    postgresql \
    postgresql-contrib \
    pgtap \
 && rm -rf /var/lib/apt/lists/*

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