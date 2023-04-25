#!/bin/sh
# Copyright (C) 2022 Greenbone AG
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

#!/bin/bash

chown -R postgres:postgres /var/lib/postgresql
chown -R postgres:postgres /var/run/postgresql
chown -R postgres:postgres /var/log/postgresql
chown -R postgres:postgres /etc/postgresql
chmod 0755 /var/lib/postgresql
chmod 0750 /var/lib/postgresql/13/main || true

exec gosu postgres "$@"
