ARG GVM_LIBS_VERSION=stable

FROM registry.community.greenbone.net/community/gvm-libs:${GVM_LIBS_VERSION} AS builder

# This will make apt-get install without question
ARG DEBIAN_FRONTEND=noninteractive

# Install
COPY . /source
WORKDIR /source

RUN sh /source/.github/install-dependencies.sh \
    /source/.github/build-dependencies.list \
    && rm -rf /var/lib/apt/lists/*
RUN cmake -DCMAKE_BUILD_TYPE=Release -B/build /source \
    && DESTDIR=/install cmake --build /build -j$(nproc) -- install

FROM registry.community.greenbone.net/community/gvm-libs:${GVM_LIBS_VERSION}

COPY --from=builder /install/ /
COPY .docker/start-postgresql.sh /usr/local/bin/start-postgresql
COPY .docker/entrypoint.sh /usr/local/bin/entrypoint

RUN --mount=type=bind,source=.github,target=/source/ \
    sh /source/install-dependencies.sh /source/runtime-dependencies.list \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /home/postgres

RUN usermod -u 104 postgres && groupmod -g 106 postgres && \
    chown -R postgres:postgres /var/lib/postgresql && \
    chown -R postgres:postgres /var/run/postgresql && \
    chown -R postgres:postgres /var/log/postgresql && \
    chown -R postgres:postgres /etc/postgresql && \
    chmod 755 /usr/local/bin/start-postgresql /usr/local/bin/entrypoint

ENTRYPOINT [ "/usr/local/bin/entrypoint" ]

CMD ["/usr/local/bin/start-postgresql"]
