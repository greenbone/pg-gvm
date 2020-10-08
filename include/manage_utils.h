/* Copyright (C) 2020 Greenbone Networks GmbH
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
 * @file manage_utils.h
 * @brief Headers for ical functions
 */

#ifndef _GVMD_MANAGE_UTILS_X_H
#define _GVMD_MANAGE_UTILS_X_H

#include <libical/ical.h>
#include <time.h>

icaltimezone *
icalendar_timezone_from_string_x (const char *);

time_t
icalendar_next_time_from_string_x (const char *, const char *, int);

time_t
icalendar_next_time_from_vcalendar_x (icalcomponent *, const char *, int);

#endif

