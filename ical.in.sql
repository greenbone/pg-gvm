CREATE FUNCTION sql_next_time_ical(ical text, tz text, p_offset text)
    RETURNS Datum
    LANGUAGE C STRICT
    AS 'MODULE_PATHNAME', $$sql_next_time_ical$$;
