# Testing the _dysize_ routine

I've been messing with installing the 'correct' code for the _dysize_ function that returns the number of days in a year. The routine is called with two possible arguments: the full four digit year and also the year as a two digit value.

There are two possible algorithms. The original:  take the value and if it's divisible by 4, then it's a leap year, and the 'correct' (see [Century Leap Year](https://en.wikipedia.org/wiki/Century_leap_year)  algorithm: if the year is exactly divisible by 4 but not by 100 or it's exactly divisible by 400.

The code in _leaptest.c_ generates a table from 1970 to
