
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
 * @file ical_utils.c
 * @brief Implements ical functions for the GVM PostgreSQL Extension.
 */

#include <limits.h>

#include "ical_utils.h"
#include "array.h"
#include "postgres.h"

/**
 * @brief Collect the times of EXDATE or RDATE properties from an VEVENT.
 * The returned GPtrArray will contain pointers to icaltimetype structs, which
 *  will be freed with g_ptr_array_free.
 *
 * @param[in]  vevent  The VEVENT component to collect times.
 * @param[in]  type    The property to get the times from.
 *
 * @return  GPtrArray with pointers to collected times or NULL on error.
 */
static array_x*
icalendar_times_from_vevent_x (icalcomponent *vevent, icalproperty_kind type)
{
  array_x* times;
  icalproperty *date_prop;

  if (icalcomponent_isa (vevent) != ICAL_VEVENT_COMPONENT
      || (type != ICAL_EXDATE_PROPERTY && type != ICAL_RDATE_PROPERTY))
    return NULL;

  times = new_array_x ();

  if (times == NULL)
    {
      return NULL;
    }

  date_prop = icalcomponent_get_first_property (vevent, type);
  while (date_prop)
    {
      icaltimetype *time;
      time = (icaltimetype*)palloc0 (sizeof (icaltimetype));
      if (time == NULL)
        {
          return NULL;
        }
      if (type == ICAL_EXDATE_PROPERTY)
        {
          *time = icalproperty_get_exdate (date_prop);
        }
      else if (type == ICAL_RDATE_PROPERTY)
        {
          struct icaldatetimeperiodtype datetimeperiod;
          datetimeperiod = icalproperty_get_rdate (date_prop);
          // Assume periods have been converted to date or datetime
          *time = datetimeperiod.time;
        }
      if (append_x(times, time) != 1)
        {
          return NULL;
        }
      date_prop = icalcomponent_get_next_property (vevent, type);
    }

  return times;
}



/**
 * @brief  Get the next or previous time from a list of RDATEs.
 *
 * @param[in]  rdates         The list of RDATEs.
 * @param[in]  tz             The icaltimezone to use.
 * @param[in]  ref_time_ical  The reference time (usually the current time).
 * @param[in]  periods_offset 0 for next, -1 for previous from/before reference.
 *
 * @return  The next or previous time as time_t.
 */
static time_t
icalendar_next_time_from_rdates_x (array_x *rdates,
                                   icaltimetype ref_time_ical,
                                   icaltimezone *tz,
                                   int periods_offset)
{
  int index;
  time_t ref_time, closest_time;
  int old_diff;

  closest_time = 0;
  ref_time = icaltime_as_timet_with_zone (ref_time_ical, tz);
  if (periods_offset < 0)
    old_diff = INT_MIN;
  else
    old_diff = INT_MAX;

  for (index = 0; index < rdates->len; index++)
    {
      icaltimetype *iter_time_ical;
      time_t iter_time;
      int time_diff;

      iter_time_ical = (icaltimetype*)rdates->data[index];
      iter_time = icaltime_as_timet_with_zone (*iter_time_ical, tz);
      time_diff = iter_time - ref_time;

      // Cases: previous (offset -1): latest before reference
      //        next     (offset  0): earliest after reference
      if ((periods_offset == -1 && time_diff < 0 && time_diff > old_diff)
          || (periods_offset == 0 && time_diff >= 0 && time_diff < old_diff))
        {
          closest_time = iter_time;
          old_diff = time_diff;
        }
    }

  return closest_time;
}


/**
 * @brief  Tests if an icaltimetype matches one in a GPtrArray.
 * When an icaltimetype is a date, only the date must match, otherwise both
 *  date and time must match.
 *
 * @param[in]  time         The icaltimetype to try to find a match of.
 * @param[in]  times_array  Array of pointers to check for a matching time.
 *
 * @return  Whether a match was found.
 */
static int
icalendar_time_matches_array_x (icaltimetype time, array_x *times_array)
{
  int found = 0;
  int index;

  if (times_array == NULL)
    return 0;

  for (index = 0;
       found == 0 && index < times_array->len;
       index++)
    {
      int compare_result;
      icaltimetype *array_time = (icaltimetype*)times_array->data[index];

      if (array_time->is_date)
        compare_result = icaltime_compare_date_only (time, *array_time);
      else
        compare_result = icaltime_compare (time, *array_time);

      if (compare_result == 0)
        found = 1;
    }
  return found;
}


/**
 * @brief Calculate the next time of a recurrence
 *
 * @param[in]  recurrence     The recurrence rule to evaluate.
 * @param[in]  dtstart        The start time of the recurrence.
 * @param[in]  reference_time The reference time (usually the current time).
 * @param[in]  tz             The icaltimezone to use.
 * @param[in]  exdates        GList of EXDATE dates or datetimes to skip.
 * @param[in]  rdates         GList of RDATE datetimes to include.
 * @param[in]  periods_offset 0 for next, -1 for previous from/before reference.
 *
 * @return  The next time.
 */
static time_t
icalendar_next_time_from_recurrence_x (struct icalrecurrencetype recurrence,
                                       icaltimetype dtstart,
                                       icaltimetype reference_time,
                                       icaltimezone *tz,
                                       array_x *exdates,
                                       array_x *rdates,
                                       int periods_offset)
{
  icalrecur_iterator *recur_iter;
  icaltimetype recur_time, prev_time, next_time;
  time_t rdates_time;

  // Start iterating over rule-based times
  recur_iter = icalrecur_iterator_new (recurrence, dtstart);
  recur_time = icalrecur_iterator_next (recur_iter);

  if (icaltime_is_null_time (recur_time))
    {
      // Use DTSTART if there are no recurrence rule times
      if (icaltime_compare (dtstart, reference_time) < 0)
        {
          prev_time = dtstart;
          next_time = icaltime_null_time ();
        }
      else
        {
          prev_time = icaltime_null_time ();
          next_time = dtstart;
        }
    }
  else
    {
      /* Handle rule-based recurrence times:
       * Get the first rule-based recurrence time, skipping ahead in case
       *  DTSTART is excluded by EXDATEs.  */

      while (icaltime_is_null_time (recur_time) == 0
             && icalendar_time_matches_array_x (recur_time, exdates))
        {
          recur_time = icalrecur_iterator_next (recur_iter);
        }

      // Set the first recur_time as either the previous or next time.
      if (icaltime_compare (recur_time, reference_time) < 0)
        {
          prev_time = recur_time;
        }
      else
        {
          prev_time = icaltime_null_time ();
        }

      /* Iterate over rule-based recurrences up to first time after
       * reference time */
      while (icaltime_is_null_time (recur_time) == 0
             && icaltime_compare (recur_time, reference_time) <= 0)
        {
          if (icalendar_time_matches_array_x (recur_time, exdates) == 0)
            prev_time = recur_time;

          recur_time = icalrecur_iterator_next (recur_iter);
        }

      // Skip further ahead if last recurrence time is in EXDATEs
      while (icaltime_is_null_time (recur_time) == 0
             && icalendar_time_matches_array_x (recur_time, exdates))
        {
          recur_time = icalrecur_iterator_next (recur_iter);
        }

      // Select last recur_time as the next_time
      next_time = recur_time;
    }

  // Get time from RDATEs
  rdates_time = icalendar_next_time_from_rdates_x (rdates, reference_time, tz,
                                                 periods_offset);

  // Select appropriate time as the RRULE time, compare it to the RDATEs time
  //  and return the appropriate time.
  if (periods_offset == -1)
    {
      time_t rrule_time;

      rrule_time = icaltime_as_timet_with_zone (prev_time, tz);
      if (rdates_time == 0 || rrule_time - rdates_time > 0)
        return rrule_time;
      else
        return rdates_time;
    }
  else
    {
      time_t rrule_time;

      rrule_time = icaltime_as_timet_with_zone (next_time, tz);
      if (rdates_time == 0 || rrule_time - rdates_time < 0)
        return rrule_time;
      else
        return rdates_time;
    }
}


/**
 * @brief Try to get a built-in libical timezone from a tzid or city name.
 *
 * @param[in]  tzid  The tzid or Olson city name.
 *
 * @return The built-in timezone if found, else NULL.
 */
icaltimezone*
icalendar_timezone_from_string_x (const char *tzid)
{
  if (tzid)
    {
      icaltimezone *tz;

      tz = icaltimezone_get_builtin_timezone_from_tzid (tzid);
      if (tz == NULL)
        tz = icaltimezone_get_builtin_timezone (tzid);
      return tz;
    }

  return NULL;
}


/**
 * @brief  Get the next or previous due time from a VCALENDAR component.
 * The VCALENDAR must have simplified with icalendar_from_string for this to
 *  work reliably.
 * The reference time is usually the current time.
 *
 * @param[in]  vcalendar       The VCALENDAR component to get the time from.
 * @param[in]  reference_time  The reference time for calculating the next time.
 * @param[in]  default_tzid    Timezone id to use if none is set in the iCal.
 * @param[in]  periods_offset  0 for next, -1 for previous from/before now.
 *
 * @return The next or previous time as a time_t.
 */
time_t
icalendar_next_time_from_vcalendar_x (icalcomponent *vcalendar,
                                      time_t reference_time,
                                      const char *default_tzid,
                                      int periods_offset)
{
  time_t now;
  icalcomponent *vevent;
  icaltimetype dtstart, dtstart_with_tz, ical_reference_time;
  icaltimezone *tz;
  icalproperty *rrule_prop;
  struct icalrecurrencetype recurrence;
  array_x *exdates, *rdates;
  time_t next_time = 0;

  // Only offsets -1 and 0 will work properly
  if (periods_offset < -1 || periods_offset > 0)
    return 0;

  // Component must be a VCALENDAR
  if (vcalendar == NULL
      || icalcomponent_isa (vcalendar) != ICAL_VCALENDAR_COMPONENT)
    return 0;

  // Process only the first VEVENT
  // Others should be removed by icalendar_from_string
  vevent = icalcomponent_get_first_component (vcalendar,
                                              ICAL_VEVENT_COMPONENT);
  if (vevent == NULL)
    return 0;

  // Get start time and timezone
  dtstart = icalcomponent_get_dtstart (vevent);
  if (icaltime_is_null_time (dtstart))
    return 0;

  tz = (icaltimezone*) icaltime_get_timezone (dtstart);
  if (tz == NULL)
    {
      tz = icalendar_timezone_from_string_x (default_tzid);
      if (tz == NULL)
        tz = icaltimezone_get_utc_timezone ();
    }

  dtstart_with_tz = dtstart;
  // Set timezone in case the original DTSTART did not have any set.
  icaltime_set_timezone (&dtstart_with_tz, tz);

  // Get current time
  ical_reference_time = icaltime_from_timet_with_zone (reference_time, 0, tz);
  // Set timezone explicitly because icaltime_current_time_with_zone doesn't.
  icaltime_set_timezone (&ical_reference_time, tz);
  if (ical_reference_time.zone == NULL)
    {
      ical_reference_time.zone = tz;
    }

  // Get EXDATEs and RDATEs
  exdates = icalendar_times_from_vevent_x (vevent, ICAL_EXDATE_PROPERTY);
  rdates = icalendar_times_from_vevent_x (vevent, ICAL_RDATE_PROPERTY);

  // Try to get the recurrence from the RRULE property
  rrule_prop = icalcomponent_get_first_property (vevent, ICAL_RRULE_PROPERTY);
  if (rrule_prop)
    recurrence = icalproperty_get_rrule (rrule_prop);
  else
    icalrecurrencetype_clear (&recurrence);

  // Calculate next time.
  next_time = icalendar_next_time_from_recurrence_x (recurrence,
                                                     dtstart_with_tz,
                                                     ical_reference_time, tz,
                                                     exdates, rdates,
                                                     periods_offset);

  // Cleanup
  free_array_x (exdates);
  free_array_x (rdates);

  return next_time;
}


/**
 * @brief  Get the next or previous due time from a VCALENDAR string.
 * The string must be a VCALENDAR simplified with icalendar_from_string for
 *  this to work reliably.
 * The reference time is usually the current time.
 *
 * @param[in]  ical_string     The VCALENDAR string to get the time from.
 * @param[in]  reference_time  The reference time for calculating the next time.
 * @param[in]  default_tzid    Timezone id to use if none is set in the iCal.
 * @param[in]  periods_offset  0 for next, -1 for previous from/before now.
 *
 * @return The next or previous time as a time_t.
 */
time_t
icalendar_next_time_from_string_x (const char *ical_string,
                                   time_t reference_time,
                                   const char *default_tzid,
                                   int periods_offset)
{
  time_t next_time;
  icalcomponent *ical_parsed;

  ical_parsed = icalcomponent_new_from_string (ical_string);
  next_time = icalendar_next_time_from_vcalendar_x (ical_parsed,
                                                    reference_time,
                                                    default_tzid,
                                                    periods_offset);
  icalcomponent_free (ical_parsed);
  return next_time;
}
