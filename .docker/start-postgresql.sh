#!/bin/sh

[ -z "$POSTGRES_USER" ] && POSTGRES_USER="gvmd"
[ -z "$POSTGRES_DATA" ] && POSTGRES_DATA="/var/lib/postgresql"

POSTGRES_DB=gvmd
POSTGRES_VERSION=13

rm -f "$POSTGRES_DATA/started"

pg_ctlcluster -o "-k /tmp" $POSTGRES_VERSION main start

createuser --host=/tmp -DRS "$POSTGRES_USER"
createdb --host=/tmp -O $POSTGRES_DB "$POSTGRES_USER"

psql --host=/tmp -d $POSTGRES_DB -c "create role dba with superuser noinherit;"
psql --host=/tmp -d $POSTGRES_DB -c "grant dba to $POSTGRES_USER;"
psql --host=/tmp -d $POSTGRES_DB -c 'create extension "uuid-ossp";'
psql --host=/tmp -d $POSTGRES_DB -c 'create extension "pgcrypto";'
psql --host=/tmp -d $POSTGRES_DB -c 'create extension "pg-gvm";'

pg_ctlcluster --foreground $POSTGRES_VERSION main stop

# Touch file, signaling startup is done
touch "$POSTGRES_DATA/started"
pg_ctlcluster --foreground $POSTGRES_VERSION main start

at_exit() {
    rm -f "$POSTGRES_DATA/started" && echo "Deleted verification file."
}

trap at_exit EXIT
