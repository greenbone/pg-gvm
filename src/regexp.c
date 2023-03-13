/* Copyright (C) 2021 Greenbone AG
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
 * @file regexp.c
 *
 * @brief This file defines functions that are available via the PostgreSQL
 * @brief extension
 */

#include "glib.h"

#include "postgres.h"
#include "fmgr.h"
#include "executor/spi.h"

/**
 * @brief Create a string from a portion of text.
 *
 * @param[in]  text_arg  Text.
 * @param[in]  length    Length to create.
 *
 * @return Freshly allocated string.
 */
static char *
textndup (text *text_arg, int length)
{
  char *ret;
  ret = palloc (length + 1);
  memcpy (ret, VARDATA (text_arg), length);
  ret[length] = 0;
  return ret;
}

/**
 * @brief Define function for Postgres.
 */
PG_FUNCTION_INFO_V1 (sql_regexp);


/**
 * @brief Return if argument 1 matches regular expression in argument 2.
 *
 * This is a callback for a SQL function of two arguments.
 *
 * @return Postgres Datum.
 */
Datum
sql_regexp (PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL (0) || PG_ARGISNULL (1))
    PG_RETURN_BOOL (0);
  else
    {
      text *string_arg, *regexp_arg;
      char *string, *regexp;
      int ret;

      regexp_arg = PG_GETARG_TEXT_P(1);
      regexp = textndup (regexp_arg, VARSIZE (regexp_arg) - VARHDRSZ);

      string_arg = PG_GETARG_TEXT_P(0);
      string = textndup (string_arg, VARSIZE (string_arg) - VARHDRSZ);

      if (g_regex_match_simple ((gchar *) regexp, (gchar *) string, 0, 0))
        ret = 1;
      else
        ret = 0;

      pfree (string);
      pfree (regexp);
      PG_RETURN_BOOL (ret);
    }
}
