/* Copyright (C) 2022 Greenbone AG
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

CREATE OR REPLACE FUNCTION next_time_ical (text, bigint, text)
    RETURNS integer
    LANGUAGE C STRICT
    AS 'MODULE_PATHNAME', $$sql_next_time_ical$$;

CREATE OR REPLACE FUNCTION next_time_ical (text, bigint, text, integer)
    RETURNS integer
    LANGUAGE C STRICT
    AS 'MODULE_PATHNAME', $$sql_next_time_ical$$;

DROP FUNCTION IF EXISTS next_time_ical (text, text);

DROP FUNCTION IF EXISTS  next_time_ical (text, text, integer);