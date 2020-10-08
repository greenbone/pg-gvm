CREATE OR REPLACE FUNCTION next_time_ical (text, text)
    RETURNS integer
    LANGUAGE C STRICT
    AS 'MODULE_PATHNAME', $$sql_next_time_ical$$;

CREATE OR REPLACE FUNCTION next_time_ical (text, text, integer)
    RETURNS integer
    LANGUAGE C STRICT
    AS 'MODULE_PATHNAME', $$sql_next_time_ical$$;
