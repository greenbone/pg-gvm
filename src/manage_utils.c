#include "manage_utils.h"

/**
 * @brief  Get the next or previous due time from a VCALENDAR component.
 * The VCALENDAR must have simplified with icalendar_from_string for this to
 *  work reliably.
 *
 * @param[in]  vcalendar       The VCALENDAR component to get the time from.
 * @param[in]  default_tzid    Timezone id to use if none is set in the iCal.
 * @param[in]  periods_offset  0 for next, -1 for previous from/before now.
 *
 * @return The next or previous time as a time_t.
 */
time_t
icalendar_next_time_from_vcalendar_x (icalcomponent *vcalendar,
                                      const char *default_tzid,
                                      int periods_offset)
{
  icalcomponent *vevent;
  icaltimetype dtstart, dtstart_with_tz, ical_now;
  icaltimezone *tz;
  icalproperty *rrule_prop;
  struct icalrecurrencetype recurrence;
  GPtrArray *exdates, *rdates;
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
      tz = icalendar_timezone_from_string (default_tzid);
      if (tz == NULL)
        tz = icaltimezone_get_utc_timezone ();
    }

  dtstart_with_tz = dtstart;
  // Set timezone in case the original DTSTART did not have any set.
  icaltime_set_timezone (&dtstart_with_tz, tz);

  // Get current time
  ical_now = icaltime_current_time_with_zone (tz);
  // Set timezone explicitly because icaltime_current_time_with_zone doesn't.
  icaltime_set_timezone (&ical_now, tz);
  if (ical_now.zone == NULL)
    {
      ical_now.zone = tz;
    }

  // Get EXDATEs and RDATEs
  exdates = icalendar_times_from_vevent (vevent, ICAL_EXDATE_PROPERTY);
  rdates = icalendar_times_from_vevent (vevent, ICAL_RDATE_PROPERTY);

  // Try to get the recurrence from the RRULE property
  rrule_prop = icalcomponent_get_first_property (vevent, ICAL_RRULE_PROPERTY);
  if (rrule_prop)
    recurrence = icalproperty_get_rrule (rrule_prop);
  else
    icalrecurrencetype_clear (&recurrence);

  // Calculate next time.
  next_time = icalendar_next_time_from_recurrence (recurrence,
                                                   dtstart_with_tz,
                                                   ical_now, tz,
                                                   exdates, rdates,
                                                   periods_offset);

  // Cleanup
  g_ptr_array_free (exdates, TRUE);
  g_ptr_array_free (rdates, TRUE);

  return next_time;
}


/**
 * @brief  Get the next or previous due time from a VCALENDAR string.
 * The string must be a VCALENDAR simplified with icalendar_from_string for
 *  this to work reliably.
 *
 * @param[in]  ical_string     The VCALENDAR string to get the time from.
 * @param[in]  default_tzid    Timezone id to use if none is set in the iCal.
 * @param[in]  periods_offset  0 for next, -1 for previous from/before now.
 *
 * @return The next or previous time as a time_t.
 */
time_t
icalendar_next_time_from_string_x (const char *ical_string,
                                   const char *default_tzid,
                                   int periods_offset)
{
  time_t next_time;
  icalcomponent *ical_parsed;

  ical_parsed = icalcomponent_new_from_string (ical_string);
  next_time = icalendar_next_time_from_vcalendar_x (ical_parsed, default_tzid,
                                                    periods_offset);
  icalcomponent_free (ical_parsed);
  return next_time;
}

