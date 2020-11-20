-- Start transaction and plan the tests.
BEGIN;

-- IMPORTANT! See https://pgtap.org/documentation.html#iloveitwhenaplancomestogether
SELECT plan(3);

-- Run the tests.
-- Test with empty input
SELECT is(hosts_contains('',''), false, 'Empty input should return false.');

SELECT is(hosts_contains('192.168.123.1-192.168.123.20, 192.168.123.30', '192.168.123.10'), true, 'Should return true');

SELECT is(hosts_contains('192.168.123.1-192.168.123.20, 192.168.123.30',  '192.168.10.20'), false, 'Should return false');

-- Finish the tests and clean up.
SELECT * FROM finish();
ROLLBACK;
