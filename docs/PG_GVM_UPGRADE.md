## PostgreSQL upgrade migrator (pg-gvm)

This image upgrades an existing PostgreSQL database to a newer major version using `pg_upgrade`.

It is meant to run **once**, before the real `pg-gvm` service starts.

---

## What this image does

* Looks for an existing PostgreSQL database in
  `/var/lib/postgresql/<major>/main`

* Detects the current PostgreSQL version (for example 13 or 15)

* Upgrades the database **directly** to the target version (default: 17)

* Creates a marker file when the upgrade finishes successfully:

  ```
  /var/lib/postgresql/.pg_upgrade_17_done
  ```

* If the marker file already exists, the container exits immediately and does nothing

This makes the migrator safe to run again.

---

## How to use the migrator image

### Build the image

Example for a local build:

```bash
docker build --no-cache -t pg-gvm-migrator:local -f ../.docker/migrator.Dockerfile ..
```
---

### Add the migrator to docker-compose

The migrator must use the **same volumes** as PostgreSQL:

* `/var/lib/postgresql` - database files
* `/var/run/postgresql` - PostgreSQL socket

Example service definition:

```yaml
services:
  pg-gvm-migrator:
    image: pg-gvm-migrator:local
    restart: "no"
    volumes:
      - psql_data_vol:/var/lib/postgresql
      - psql_socket_vol:/var/run/postgresql
    environment:
      TARGET_PG_MAJOR: "17"
      POSTGRES_CLUSTER: "main"
      POSTGRES_DATA: /var/lib/postgresql
    healthcheck:
      test: ["CMD-SHELL", "test -f /var/lib/postgresql/.pg_upgrade_17_done"]
      interval: 2s
      timeout: 2s
      retries: 180
```

The healthcheck waits until the upgrade marker file exists.

---

### Start pg-gvm only after the upgrade

The real `pg-gvm` service must wait until the migrator finishes successfully.

Example:

```yaml
  pg-gvm:
    image: <pg-gvm image for PostgreSQL 17>
    restart: on-failure
    volumes:
      - psql_data_vol:/var/lib/postgresql
      - psql_socket_vol:/var/run/postgresql
    depends_on:
      pg-gvm-migrator:
        condition: service_completed_successfully
```

This ensures:

* The migrator runs first
* The database upgrade finishes successfully
* `pg-gvm` starts using the upgraded database

---

## What happens when you run `docker compose up`

* If the database needs an upgrade, the migrator runs `pg_upgrade` and exits
* If the database is already upgraded, the migrator exits immediately
* `pg-gvm` starts only after the migrator finishes successfully

---

## Important notes

* The migrator upgrades **directly** from the detected version to the target version (for example 13 to 17)
* All required PostgreSQL versions and extension libraries must be present in the image
* If the upgrade fails, the container prints error logs and exits with a non-zero status
* You can safely restart Docker Compose; the marker file prevents re-running the upgrade
* The image uses **`tini`** to forward shutdown signals correctly and clean up child processes if the container is stopped during an upgrade

---

