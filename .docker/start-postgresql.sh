#!/bin/sh

[ -z "$GVMD_USER" ] && GVMD_USER="gvmd"
[ -z "$PGRES_DATA"] && PGRES_DATA="/var/lib/postgresql"

rm -f $PGRES_DATA/started

pg_ctlcluster -o "-k /tmp" 13 main start

createuser --host=/tmp -DRS "$GVMD_USER"
createdb --host=/tmp -O gvmd "$GVMD_USER"

psql --host=/tmp -d gvmd -c "create role dba with superuser noinherit;"
psql --host=/tmp -d gvmd -c "grant dba to $GVMD_USER;"
psql --host=/tmp -d gvmd -c 'create extension "uuid-ossp";'
psql --host=/tmp -d gvmd -c 'create extension "pgcrypto";'
psql --host=/tmp -d gvmd -c 'create extension "pg-gvm";'

pg_ctlcluster --foreground 13 main stop
# Touch file, signaling startup is done
touch $PGRES_DATA/started
pg_ctlcluster --foreground 13 main start
trap 'rm -f $PGRES_DATA/started && echo "Deleted verification file."' EXIT
