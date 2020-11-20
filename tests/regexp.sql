-- Start transaction and plan the tests.
BEGIN;

-- IMPORTANT! See https://pgtap.org/documentation.html#iloveitwhenaplancomestogether
SELECT plan(3);

-- Run the tests.
SELECT ok(regexp ('abc', '^[a-z]+$'), 'Should match!');
SELECT is(regexp ('123', '^[a-z]+$'), false, 'Should not match');
SELECT is(regexp ('123', '^[a-z+$'), false, 'Should return false because regex is invalid');

-- Finish the tests and clean up.
SELECT * FROM finish();
ROLLBACK;
