/* Copyright (C) 2020-2022 Greenbone Networks GmbH
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
 * @file ical.c
 *
 * @brief This file defines functions that are available via the PostgreSQL
 * @brief extension
 */

#include "ical_utils.h"

#include "postgres.h"
#include "fmgr.h"
#include "executor/spi.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

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
PG_FUNCTION_INFO_V1 (sql_next_time_ical);

/**
 * @brief Get the next time given schedule times.
 *
 * This is a callback for a SQL function of one to three arguments.
 *
 * @return Postgres Datum.
 */
Datum
sql_next_time_ical (PG_FUNCTION_ARGS)
{
  char *ical_string, *zone;
  int64 reference_time;
  int periods_offset;
  int32 ret;

  if (PG_NARGS() < 1 || PG_ARGISNULL (0))
    {
      PG_RETURN_NULL ();
    }
  else
    {
      text* ical_string_arg;
      ical_string_arg = PG_GETARG_TEXT_P (0);
      ical_string = textndup (ical_string_arg,
                              VARSIZE (ical_string_arg) - VARHDRSZ);
    }

  if (PG_NARGS() < 2 || PG_ARGISNULL (1))
    reference_time = 0;
  else
    reference_time = PG_GETARG_INT64 (1);
    
  if (PG_NARGS() < 3 || PG_ARGISNULL (2))
    zone = NULL;
  else
    {
      text* timezone_arg;
      timezone_arg = PG_GETARG_TEXT_P (2);
      zone = textndup (timezone_arg, VARSIZE (timezone_arg) - VARHDRSZ);
    }

  if (PG_NARGS() < 4)
    periods_offset = 0;
  else
    periods_offset = PG_GETARG_INT32 (3);

  ret = icalendar_next_time_from_string_x (ical_string, reference_time, zone,
                                           periods_offset);
  if (ical_string)
    pfree (ical_string);
  if (zone)
    pfree (zone);
  PG_RETURN_INT32 (ret);
}
