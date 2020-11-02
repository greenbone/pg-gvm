/* Copyright (C) 2020 Greenbone Networks GmbH
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file hosts.c
 *
 * @brief This file defines functions that are available via the PostgreSQL
 * @brief extension
 */

#include "hosts.h"

#include "postgres.h"
#include "fmgr.h"
#include "executor/spi.h"
#include "glib.h"

#include <gvm/base/hosts.h>

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

/**
 * @brief Get the maximum number of hosts.
 *
 * @return The maximum number of hosts.
 */
static int
get_max_hosts_x ()
{
  int ret;
  int max_hosts = 4095;
  SPI_connect ();
  ret = SPI_exec ("SELECT coalesce ((SELECT value FROM meta"
                  "                  WHERE name = 'max_hosts'),"
                  "                 '4095');", /* Same as MANAGE_MAX_HOSTS. */
                  1); /* Max 1 row returned. */
  if (SPI_processed > 0 && ret > 0 && SPI_tuptable != NULL)
    {
      char *cell;

      cell = SPI_getvalue (SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 1);
      elog (DEBUG1, "cell: %s", cell);
      if (cell)
        max_hosts = atoi (cell);
    }
  elog (DEBUG1, "done");
  SPI_finish ();

  return max_hosts;
}

/**
 * @brief Define function for Postgres.
 */
PG_FUNCTION_INFO_V1 (sql_max_hosts);


/**
 * @brief Return number of hosts.
 *
 * This is a callback for a SQL function of two arguments.
 *
 * @return Postgres Datum.
 */
Datum
sql_max_hosts (PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL (0))
    PG_RETURN_INT32 (0);
  else
    {
      text *hosts_arg;
      char *hosts, *exclude;
      int ret, max_hosts;

      hosts_arg = PG_GETARG_TEXT_P (0);
      hosts = textndup (hosts_arg, VARSIZE (hosts_arg) - VARHDRSZ);
      if (PG_ARGISNULL (1))
        {
          exclude = palloc (1);
          exclude[0] = 0;
        }
      else
        {
          text *exclude_arg;
          exclude_arg = PG_GETARG_TEXT_P (1);
          exclude = textndup (exclude_arg, VARSIZE (exclude_arg) - VARHDRSZ);
        }

      max_hosts = get_max_hosts_x ();
      ret = manage_count_hosts_max (hosts, exclude, max_hosts);
      pfree (hosts);
      pfree (exclude);
      PG_RETURN_INT32 (ret);
    }
}


/**
 * @brief Define function for Postgres.
 */
PG_FUNCTION_INFO_V1 (sql_hosts_contains);

/**
 * @brief Return if argument 1 matches regular expression in argument 2.
 *
 * This is a callback for a SQL function of two arguments.
 *
 * @return Postgres Datum.
 */
Datum
sql_hosts_contains (PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL (0) || PG_ARGISNULL (1))
    PG_RETURN_BOOL (0);
  else
    {
      text *hosts_arg, *find_host_arg;
      char *hosts, *find_host;
      int max_hosts, ret;

      hosts_arg = PG_GETARG_TEXT_P(0);
      hosts = textndup (hosts_arg, VARSIZE (hosts_arg) - VARHDRSZ);

      find_host_arg = PG_GETARG_TEXT_P(1);
      find_host = textndup (find_host_arg, VARSIZE (find_host_arg) - VARHDRSZ);

      max_hosts = get_max_hosts_x ();

      if (hosts_str_contains ((gchar *) hosts, (gchar *) find_host,
                              max_hosts))
        ret = 1;
      else
        ret = 0;

      pfree (hosts);
      pfree (find_host);
      PG_RETURN_BOOL (ret);
    }
}
