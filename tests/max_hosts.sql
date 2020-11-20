-- Start transaction and plan the tests.
BEGIN;

-- IMPORTANT! See https://pgtap.org/documentation.html#iloveitwhenaplancomestogether
SELECT plan(2);

-- Run the tests.
-- Test with empty input
SELECT is(max_hosts('192.168.123.1-192.168.123.20, 192.168.123.30-192.168.123.34', ''), 25, 'Value should be 25');
SELECT is(max_hosts('192.168.123.1-192.168.123.20, 192.168.123.30-192.168.123.34', '192.168.123.10'), 24, 'Value should be 24');

-- Finish the tests and clean up.
SELECT * FROM finish();
ROLLBACK;
