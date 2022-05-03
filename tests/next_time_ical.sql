-- Start transaction and plan the tests.
BEGIN;

-- IMPORTANT! See https://pgtap.org/documentation.html#iloveitwhenaplancomestogether
SELECT plan(10);

-- Function to calculate the test timestamps based on current time
--  PostgreSQL internal date-time functions.
CREATE OR REPLACE FUNCTION next_test_time (integer, timestamp with time zone)
RETURNS integer AS $$
DECLARE
    current_year integer;
    current_month integer;
    next_time timestamp with time zone;
BEGIN
    current_year = EXTRACT (year FROM $2);
    current_month = EXTRACT (month FROM $2);

    -- First guess based on month
    IF current_month <= 5 THEN
      next_time
        = make_timestamptz(current_year, 5, 21, 3, 7, 0, 'Europe/Berlin');
    ELSE
      next_time
        = make_timestamptz(current_year, 11, 21, 3, 7, 0, 'Europe/Berlin');
    END IF;

    -- If 03:07:00 on the 21st has already passed in May / November,
    --  add 6 months.
    IF $2 > next_time THEN
      next_time 
        = (next_time AT TIME ZONE 'Europe/Berlin' + interval '6 months')
            AT TIME ZONE 'Europe/Berlin';
    END IF;

    -- Apply offset
    next_time 
      = (next_time AT TIME ZONE 'Europe/Berlin' + ($1 * interval '6 months'))
          AT TIME ZONE 'Europe/Berlin';

    RETURN extract (EPOCH FROM (next_time AT TIME ZONE 'UTC'));
END;
$$ LANGUAGE plpgsql;

--
-- Test the date test model function
--

-- 2020-02-02T12:00:00 CET -> 2020-05-21T03:07:00 CEST
SELECT is (next_test_time (0, to_timestamp (1580641200)), 1590023220);

-- 2020-05-21T03:00:00 CEST -> 2020-05-21T03:07:00 CEST
SELECT is (next_test_time (0, to_timestamp (1590022800)), 1590023220);

-- 2020-05-21T03:10:00 CEST -> 2020-11-21T03:07:00 CET
SELECT is (next_test_time (0, to_timestamp (1590023400)), 1605924420);

-- 2020-08-21T00:00:00 CEST -> 2020-11-21T03:07:00 CET
SELECT is (next_test_time (0, to_timestamp (1597960800)), 1605924420);

-- 2020-11-21T03:00:00 CET -> 2020-11-21T03:07:00 CET
SELECT is (next_test_time (0, to_timestamp (1605924000)), 1605924420);

-- 2020-11-21T03:10:00 CET -> 2021-05-21T03:07:00 CEST
SELECT is (next_test_time (0, to_timestamp (1605924600)), 1621559220);

-- 2020-12-12T11:11:11 CET -> 2021-05-21T03:07:00 CEST
SELECT is (next_test_time (0, to_timestamp (1607767871)), 1621559220);

--
-- Run the tests for the actual iCalendar function.
--

-- Test without offset parameter
SELECT is (next_time_ical('BEGIN:VCALENDAR
VERSION:2.0
PRODID:-//Greenbone.net//NONSGML Greenbone Security Manager 
 20.8+alpha~git-053b4bcb-timezone-ical//EN
BEGIN:VTIMEZONE
TZID:/freeassociation.sourceforge.net/Europe/Berlin
X-LIC-LOCATION:Europe/Berlin
BEGIN:DAYLIGHT
TZNAME:CEST
DTSTART:19810329T020000
TZOFFSETFROM:+0100
TZOFFSETTO:+0200
RRULE:FREQ=YEARLY;BYDAY=-1SU;BYMONTH=3
END:DAYLIGHT
BEGIN:STANDARD
TZNAME:CET
DTSTART:19961025T030000
TZOFFSETFROM:+0200
TZOFFSETTO:+0100
RRULE:FREQ=YEARLY;BYDAY=-1SU;BYMONTH=10
END:STANDARD
END:VTIMEZONE
BEGIN:VEVENT
DTSTART;TZID=/freeassociation.sourceforge.net/Europe/Berlin:
 20100521T030700
DURATION:PT0S
RRULE:FREQ=MONTHLY;INTERVAL=6;BYMONTHDAY=21
UID:8c022087-e10a-462e-a1af-65559601a0db
DTSTAMP:20200615T161125Z
END:VEVENT
END:VCALENDAR', EXTRACT (EPOCH from now())::bigint, 'Europe/Berlin'),
next_test_time (0, now ()),
'Calculation was wrong');

-- Test with offset parameter set to 0
SELECT is (next_time_ical('BEGIN:VCALENDAR
VERSION:2.0
PRODID:-//Greenbone.net//NONSGML Greenbone Security Manager 
 20.8+alpha~git-053b4bcb-timezone-ical//EN
BEGIN:VTIMEZONE
TZID:/freeassociation.sourceforge.net/Europe/Berlin
X-LIC-LOCATION:Europe/Berlin
BEGIN:DAYLIGHT
TZNAME:CEST
DTSTART:19810329T020000
TZOFFSETFROM:+0100
TZOFFSETTO:+0200
RRULE:FREQ=YEARLY;BYDAY=-1SU;BYMONTH=3
END:DAYLIGHT
BEGIN:STANDARD
TZNAME:CET
DTSTART:19961025T030000
TZOFFSETFROM:+0200
TZOFFSETTO:+0100
RRULE:FREQ=YEARLY;BYDAY=-1SU;BYMONTH=10
END:STANDARD
END:VTIMEZONE
BEGIN:VEVENT
DTSTART;TZID=/freeassociation.sourceforge.net/Europe/Berlin:
 20100521T030700
DURATION:PT0S
RRULE:FREQ=MONTHLY;INTERVAL=6;BYMONTHDAY=21
UID:8c022087-e10a-462e-a1af-65559601a0db
DTSTAMP:20200615T161125Z
END:VEVENT
END:VCALENDAR', EXTRACT (EPOCH from now())::bigint, 'Europe/Berlin', 0),
next_test_time (0, now ()),
'Calculation was wrong');

-- Test with offset parameter set to -1
SELECT is (next_time_ical('BEGIN:VCALENDAR
VERSION:2.0
PRODID:-//Greenbone.net//NONSGML Greenbone Security Manager 
 20.8+alpha~git-053b4bcb-timezone-ical//EN
BEGIN:VTIMEZONE
TZID:/freeassociation.sourceforge.net/Europe/Berlin
X-LIC-LOCATION:Europe/Berlin
BEGIN:DAYLIGHT
TZNAME:CEST
DTSTART:19810329T020000
TZOFFSETFROM:+0100
TZOFFSETTO:+0200
RRULE:FREQ=YEARLY;BYDAY=-1SU;BYMONTH=3
END:DAYLIGHT
BEGIN:STANDARD
TZNAME:CET
DTSTART:19961025T030000
TZOFFSETFROM:+0200
TZOFFSETTO:+0100
RRULE:FREQ=YEARLY;BYDAY=-1SU;BYMONTH=10
END:STANDARD
END:VTIMEZONE
BEGIN:VEVENT
DTSTART;TZID=/freeassociation.sourceforge.net/Europe/Berlin:
 20100521T030700
DURATION:PT0S
RRULE:FREQ=MONTHLY;INTERVAL=6;BYMONTHDAY=21
UID:8c022087-e10a-462e-a1af-65559601a0db
DTSTAMP:20200615T161125Z
END:VEVENT
END:VCALENDAR', EXTRACT (EPOCH from now())::bigint, 'Europe/Berlin', -1),
next_test_time (-1, now ()),
'Calculation was wrong');

-- Finish the tests and clean up.
SELECT * FROM finish();

DROP FUNCTION next_test_time (integer, timestamp with time zone);

ROLLBACK;
