#!/bin/sh
# Copyright (C) 2026 Greenbone AG
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -e

: "${POSTGRES_DATA:=/var/lib/postgresql}"
: "${POSTGRES_CLUSTER:=main}"
: "${TARGET_PG_MAJOR:=17}"
: "${PGAUTO_ONESHOT:=yes}"
: "${PGAUTO_REINDEX:=yes}"
: "${LINK_MODE:=1}"
: "${KEEP_OLD_DATADIR:=0}"
: "${INITDB_LOCALE:=C}"
: "${INITDB_ENCODING:=UTF8}"

SOCKET_DIR="/var/run/postgresql"
UPGRADE_MARKER="${POSTGRES_DATA}/.pg_upgrade_${TARGET_PG_MAJOR}_done"

run_as_postgres() {
  su -s /bin/sh postgres -c "cd /tmp && $1"
}

detect_major() {
  for f in "$POSTGRES_DATA"/*/"$POSTGRES_CLUSTER"/PG_VERSION; do
    [ -f "$f" ] || continue
    cat "$f"
    return 0
  done
  return 1
}

ensure_tmp() {
  mkdir -p /tmp
  chmod 1777 /tmp
  mkdir -p /tmp/pg-upgrade
  chown -R postgres:postgres /tmp/pg-upgrade
  chmod 700 /tmp/pg-upgrade
}

ensure_socket_dir() {
  mkdir -p "$SOCKET_DIR"
  chown postgres:postgres "$SOCKET_DIR"
  chmod 2775 "$SOCKET_DIR"
}

mark_done() {
  touch "$UPGRADE_MARKER"
}

inject_conf_if_missing() {
  datadir="$1"
  conf="$datadir/postgresql.conf"
  hba="$datadir/pg_hba.conf"

  if [ ! -f "$conf" ]; then
    cat > "$conf" <<EOF
unix_socket_directories = '$SOCKET_DIR'
listen_addresses = ''
port = 5432
EOF
    chown postgres:postgres "$conf"
  fi

  if [ ! -f "$hba" ]; then
    cat > "$hba" <<EOF
local all all trust
EOF
    chown postgres:postgres "$hba"
  fi
}

cleanup_pid_files() {
  datadir="$1"
  rm -f "$datadir/postmaster.pid" "$datadir/postmaster.opts" 2>/dev/null || true
}

ensure_clean_shutdown() {
  old="$1"
  old_datadir="$2"
  old_bindir="$3"
  log="/tmp/pg-upgrade/pg${old}.log"

  cleanup_pid_files "$old_datadir"
  inject_conf_if_missing "$old_datadir"

  if ! run_as_postgres "\"$old_bindir/pg_ctl\" -D \"$old_datadir\" -l \"$log\" -o \"-k $SOCKET_DIR -h '' -c config_file=$old_datadir/postgresql.conf\" start"; then
    echo "ERROR: could not start old cluster $old for recovery. Log follows:"
    run_as_postgres "tail -n 200 \"$log\" || true"
    exit 1
  fi

  run_as_postgres "\"$old_bindir/pg_ctl\" -D \"$old_datadir\" -m fast stop"
}

init_new_cluster() {
  new_datadir="$1"
  new_bindir="$2"

  rm -rf "$new_datadir"
  mkdir -p "$new_datadir"
  chown -R postgres:postgres "$(dirname "$new_datadir")"

  run_as_postgres "\"$new_bindir/initdb\" -D \"$new_datadir\" --locale=\"$INITDB_LOCALE\" --encoding=\"$INITDB_ENCODING\""
}

run_pg_upgrade() {
  old_bindir="$1"
  new_bindir="$2"
  old_datadir="$3"
  new_datadir="$4"
  new_major="$5"

  if [ "$LINK_MODE" = "1" ]; then
    LINK_FLAG="--link"
  else
    LINK_FLAG=""
  fi

  export LD_LIBRARY_PATH="/usr/lib/postgresql/$new_major/lib:${LD_LIBRARY_PATH:-}"

  rm -f /tmp/loadable_libraries.txt /tmp/pg_upgrade_internal.log 2>/dev/null || true

  if ! run_as_postgres "\"$new_bindir/pg_upgrade\" $LINK_FLAG \
    --old-bindir=\"$old_bindir\" --new-bindir=\"$new_bindir\" \
    --old-datadir=\"$old_datadir\" --new-datadir=\"$new_datadir\""; then

    echo "ERROR: pg_upgrade failed."

    if [ -f /tmp/loadable_libraries.txt ]; then
      echo "Missing libraries (from /tmp/loadable_libraries.txt):"
      cat /tmp/loadable_libraries.txt || true
    fi

    if [ -f /tmp/pg_upgrade_internal.log ]; then
      echo "pg_upgrade_internal.log (tail):"
      tail -n 200 /tmp/pg_upgrade_internal.log || true
    fi

    exit 1
  fi
}

maybe_reindex() {
  if [ "$PGAUTO_REINDEX" != "yes" ]; then
    return 0
  fi
  if [ -f "$POSTGRES_DATA/analyze_new_cluster.sh" ]; then
    cp -f "$POSTGRES_DATA/analyze_new_cluster.sh" /tmp/analyze_new_cluster.sh 2>/dev/null || true
    run_as_postgres "sh /tmp/analyze_new_cluster.sh" || true
  fi
}

finalize_debian_cluster_conf() {
  new_major="$1"
  cluster="$2"

  conf="/etc/postgresql/${new_major}/${cluster}/postgresql.conf"
  hba="/etc/postgresql/${new_major}/${cluster}/pg_hba.conf"

  ensure_socket_dir

  if [ -f "$conf" ]; then
    if grep -q '^[#[:space:]]*unix_socket_directories' "$conf"; then
      sed -i "s|^[#[:space:]]*unix_socket_directories.*|unix_socket_directories = '$SOCKET_DIR'|" "$conf"
    else
      echo "unix_socket_directories = '$SOCKET_DIR'" >> "$conf"
    fi

    if grep -q '^[#[:space:]]*listen_addresses' "$conf"; then
      sed -i "s|^[#[:space:]]*listen_addresses.*|listen_addresses = ''|" "$conf"
    else
      echo "listen_addresses = ''" >> "$conf"
    fi
  fi

  if [ -f "$hba" ] && ! grep -q '^local[[:space:]]\+all[[:space:]]\+all' "$hba"; then
    echo "local all all trust" >> "$hba"
  fi
}

do_upgrade_once() {
  old="$1"
  new="$2"

  old_datadir="$POSTGRES_DATA/$old/$POSTGRES_CLUSTER"
  new_datadir="$POSTGRES_DATA/$new/$POSTGRES_CLUSTER"
  old_bindir="/usr/lib/postgresql/$old/bin"
  new_bindir="/usr/lib/postgresql/$new/bin"

  echo "Upgrading cluster $POSTGRES_CLUSTER: $old -> $new"

  [ -f "$old_datadir/PG_VERSION" ] || { echo "ERROR: missing $old_datadir/PG_VERSION"; exit 1; }
  [ -x "$old_bindir/pg_ctl" ] || { echo "ERROR: missing $old_bindir/pg_ctl"; exit 1; }
  [ -x "$new_bindir/initdb" ] || { echo "ERROR: missing $new_bindir/initdb"; exit 1; }
  [ -x "$new_bindir/pg_upgrade" ] || { echo "ERROR: missing $new_bindir/pg_upgrade"; exit 1; }

  chown -R postgres:postgres "$POSTGRES_DATA"

  ensure_socket_dir
  ensure_clean_shutdown "$old" "$old_datadir" "$old_bindir"
  init_new_cluster "$new_datadir" "$new_bindir"

  run_pg_upgrade "$old_bindir" "$new_bindir" "$old_datadir" "$new_datadir" "$new"

  maybe_reindex
  finalize_debian_cluster_conf "$new" "$POSTGRES_CLUSTER"

  if [ "$KEEP_OLD_DATADIR" = "0" ]; then
    rm -rf "$POSTGRES_DATA/$old"
  fi
}

main() {
  ensure_tmp
  ensure_socket_dir

  if [ -f "$UPGRADE_MARKER" ]; then
    echo "Upgrade marker present ($UPGRADE_MARKER) — skipping upgrade."
    exit 0
  fi

  CURRENT="$(detect_major || true)"
  if [ -z "$CURRENT" ]; then
    echo "No existing cluster found in $POSTGRES_DATA. Nothing to upgrade."
    mark_done
    exit 0
  fi

  echo "Detected current major: $CURRENT"
  echo "Target major: $TARGET_PG_MAJOR"

  if [ "$CURRENT" = "$TARGET_PG_MAJOR" ]; then
    echo "Already at target major."
    mark_done
    exit 0
  fi

  do_upgrade_once "$CURRENT" "$TARGET_PG_MAJOR"

  echo "Upgrade completed: $CURRENT -> $TARGET_PG_MAJOR"
  mark_done
}

main
