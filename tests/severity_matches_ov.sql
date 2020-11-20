-- Start transaction and plan the tests.
BEGIN;

-- IMPORTANT! See https://pgtap.org/documentation.html#iloveitwhenaplancomestogether
SELECT plan(7);

-- Run the tests.
SELECT is(severity_matches_ov (null, null), false, 'Input of NULL value should always return false');
SELECT ok(severity_matches_ov (5.0, null), 'Input of NULL in second parameter should return true for overrides');
SELECT is(severity_matches_ov (5.0, 10.0), false, 'Should return `false` because the result severity is lower than what is required by the override');
SELECT ok(severity_matches_ov (5.0, 5.0), 'Should return `true` because the result severity is as high as required by the override or higher');
SELECT ok(severity_matches_ov (10.0, 5.0), 'Should return `true` because the result severity is as high as required by the override or higher');

-- Operator dows not exist: IF $1 <= 0 THEN    RETURN $1 == $2; -> Should be IF $1 <= 0 THEN    RETURN $1 = $2;
--                                                       ^^                                               ^
-- TODO: Fix this and activate the following test cases! Don't forget to increase plan()!
--
SELECT ok(severity_matches_ov (0.0, 0.0), 'Should return `true` because the two severity scores match exactly');
SELECT is(severity_matches_ov (-1.0, 0.0), false, 'Should return `false` because the result has a severity <= 0.0 and does not match exactly');

-- Finish the tests and clean up.
SELECT * FROM finish();
ROLLBACK;
