ARG VERSION=unstable

FROM greenbone/gvm-libs:${VERSION} as builder

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

FROM greenbone/gvm-libs:${VERSION}

COPY --from=builder /install/ /
COPY .docker/start-postgresql.sh /usr/local/bin/start-postgres

RUN chmod 755 /usr/local/bin/start-postgres

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    libgpgme11 \
    libical3 \
    libpq5 \
    postgresql-13 \
    postgresql-client-13 \
    postgresql-client-common && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /home/postgres

RUN usermod -u 104 postgres && groupmod -g 106 postgres

RUN chown -R postgres:postgres /var/lib/postgresql
RUN chown -R postgres:postgres /var/run/postgresql
RUN chown -R postgres:postgres /var/log/postgresql
RUN chown -R postgres:postgres /etc/postgresql

RUN sed -i 's/peer/trust/' /etc/postgresql/13/main/pg_hba.conf

USER postgres

CMD ["/usr/local/bin/start-postgresql"]
