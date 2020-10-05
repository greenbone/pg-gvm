#include "manage_utils.h"

#include "postgres.h"

/* Copyright (C) 2014-2018 Greenbone Networks GmbH
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


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
PG_FUNCTION_INFO_V1 (sql_next_time);

/**
 * @brief Dummy function to allow restoring gvmd-9.0 dumps.
 *
 * @deprecated This function will be removed once direct migration
 *             compatibility with gvmd 9.0 is no longer required
 *
 * @return Postgres NULL Datum.
 */
 __attribute__((deprecated))
Datum
sql_next_time (PG_FUNCTION_ARGS)
{
  PG_RETURN_NULL ();
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
    zone = NULL;
  else
    {
      text* timezone_arg;
      timezone_arg = PG_GETARG_TEXT_P (1);
      zone = textndup (timezone_arg, VARSIZE (timezone_arg) - VARHDRSZ);
    }

  if (PG_NARGS() < 3)
    periods_offset = 0;
  else
    periods_offset = PG_GETARG_INT32 (2);

  ret = icalendar_next_time_from_string (ical_string, zone,
                                         periods_offset);
  if (ical_string)
    pfree (ical_string);
  if (zone)
    pfree (zone);
  PG_RETURN_INT32 (ret);
}
