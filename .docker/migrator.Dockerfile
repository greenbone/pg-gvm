ARG GVM_LIBS_VERSION=stable

FROM registry.community.greenbone.net/community/gvm-libs:${GVM_LIBS_VERSION} AS pg_builder

ARG DEBIAN_FRONTEND=noninteractive
ARG PG_MAJORS="14 15 16 17"

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

WORKDIR /source
COPY . /source

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates curl gnupg \
    postgresql-common \
    build-essential cmake git pkg-config \
    libglib2.0-dev libgnutls28-dev libical-dev \
  && rm -rf /var/lib/apt/lists/*

RUN set -eux; \
  . /etc/os-release; \
  echo "Detected builder distro codename: ${VERSION_CODENAME}"; \
  curl -fsSL https://www.postgresql.org/media/keys/ACCC4CF8.asc \
    | gpg --dearmor -o /usr/share/keyrings/postgresql.gpg; \
  echo "deb [signed-by=/usr/share/keyrings/postgresql.gpg] https://apt.postgresql.org/pub/repos/apt ${VERSION_CODENAME}-pgdg main" \
    > /etc/apt/sources.list.d/pgdg.list

RUN set -eux; \
  apt-get update; \
  for v in ${PG_MAJORS}; do \
    echo "==== Building for PostgreSQL ${v} ===="; \
    apt-get install -y --no-install-recommends postgresql-server-dev-${v}; \
    rm -rf /build && mkdir -p /build; \
    export PATH="/usr/lib/postgresql/${v}/bin:${PATH}"; \
    command -v pg_config; \
    pg_config --version; \
    cmake -S /source -B /build -DCMAKE_BUILD_TYPE=Release; \
    DESTDIR="/install-${v}" cmake --build /build -j"$(nproc)" --target install; \
  done; \
  rm -rf /var/lib/apt/lists/*


FROM registry.community.greenbone.net/community/gvm-libs:${GVM_LIBS_VERSION}

ARG DEBIAN_FRONTEND=noninteractive

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates curl gnupg \
    postgresql-common \
    tini \
    libglib2.0-0 libgnutls30 libical3 \
  && rm -rf /var/lib/apt/lists/*

RUN set -eux; \
  . /etc/os-release; \
  echo "Runtime distro codename: ${VERSION_CODENAME}"; \
  curl -fsSL https://www.postgresql.org/media/keys/ACCC4CF8.asc \
    | gpg --dearmor -o /usr/share/keyrings/postgresql.gpg; \
  echo "deb [signed-by=/usr/share/keyrings/postgresql.gpg] https://apt.postgresql.org/pub/repos/apt ${VERSION_CODENAME}-pgdg main" \
    > /etc/apt/sources.list.d/pgdg.list

RUN apt-get update && apt-get install -y --no-install-recommends \
    postgresql-13 postgresql-14 postgresql-15 postgresql-16 postgresql-17 \
    postgresql-client-13 postgresql-client-14 postgresql-client-15 postgresql-client-16 postgresql-client-17 \
  && rm -rf /var/lib/apt/lists/*

COPY --from=pg_builder /install-14/ /
COPY --from=pg_builder /install-15/ /
COPY --from=pg_builder /install-16/ /
COPY --from=pg_builder /install-17/ /

COPY .docker/upgrade.sh /usr/local/bin/upgrade.sh
RUN chmod 0755 /usr/local/bin/upgrade.sh

RUN groupmod -g 106 postgres && usermod -u 104 -g 106 postgres
HEALTHCHECK --interval=2s --timeout=2s --start-period=10s --retries=180 \
  CMD test -f /var/lib/postgresql/.pg_upgrade_done || exit 1

ENTRYPOINT ["/usr/bin/tini","--"]

CMD ["/usr/local/bin/upgrade.sh"]
