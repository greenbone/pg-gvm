
#ifndef _GVMD_MANAGE_UTILS_X_H
#define _GVMD_MANAGE_UTILS_X_H

#include <glib.h>
#include <libical/ical.h>
#include <time.h>

time_t
icalendar_next_time_from_string_x (const char *, const char *, int);

time_t
icalendar_next_time_from_vcalendar_x (icalcomponent *, const char *, int);

#endif

