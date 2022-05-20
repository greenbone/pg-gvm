ARG VERSION=stable

FROM greenbone/gvmd:${VERSION} as gvmd
FROM greenbone/gvm-libs:${VERSION}

COPY --from=gvmd /usr/local/lib/libgvm-pg-server.so /usr/local/lib/
COPY .docker/start-postgresql.sh /usr/local/bin/start-postgresql
COPY .docker/entrypoint.sh /usr/local/bin/entrypoint

RUN chmod 755 /usr/local/bin/start-postgresql /usr/local/bin/entrypoint

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    gosu \
    libgpgme11 \
    libical3 \
    libpq5 \
    postgresql-13 \
    postgresql-client-13 \
    postgresql-client-common && \
    rm -rf /var/lib/apt/lists/*

RUN ldconfig

WORKDIR /home/postgres

RUN usermod -u 104 postgres && groupmod -g 106 postgres

RUN chown -R postgres:postgres /var/lib/postgresql
RUN chown -R postgres:postgres /var/run/postgresql
RUN chown -R postgres:postgres /var/log/postgresql
RUN chown -R postgres:postgres /etc/postgresql

RUN sed -i 's/peer/trust/' /etc/postgresql/13/main/pg_hba.conf

ENTRYPOINT [ "/usr/local/bin/entrypoint" ]

CMD ["/usr/local/bin/start-postgresql"]
