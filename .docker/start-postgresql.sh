#!/bin/sh

[ -z "$GVMD_USER" ] && GVMD_USER="gvmd"
[ -z "$PGRES_DATA"] && PGRES_DATA="/var/lib/postgresql"

pg_ctlcluster 13 main start

createuser -DRS "$GVMD_USER"
createdb -O gvmd "$GVMD_USER"

psql -d gvmd -c "create role dba with superuser noinherit;"
psql -d gvmd -c "grant dba to $GVMD_USER;"
psql -d gvmd -c 'create extension "uuid-ossp";'
psql -d gvmd -c 'create extension "pgcrypto";'
psql -d gvmd -c 'create extension "pg-gvm";'

pg_ctlcluster --foreground 13 main stop
pg_ctlcluster --foreground 13 main start

# Touch file, signaling startup is done
touch $PGRES_DATA/done