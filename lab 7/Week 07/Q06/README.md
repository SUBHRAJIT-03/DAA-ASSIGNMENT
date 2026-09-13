## Q06 - The Best Time to Be Alive

## How the Code Works
The program asks the user to choose either manual input or random generation of birth and death years. For each person, the code creates two events:
- birth year: adds 1 to the active population count
- death year: subtracts 1 after the person is no longer alive

The events are sorted by year, and then a sweep through the sorted list is performed to find the highest population count at any year.

## Maths / Logic Behind This
For each person with birth year `b` and death year `d`, the program treats the interval as:

- `+1` at year `b`
- `-1` at year `d + 1`

This ensures the person is counted as alive during the interval `[b, d]` and is removed after death. The year with the highest cumulative count is the best year to have been alive.

## Complexity Analysis
- Time Complexity: O(n log n)
- Space Complexity: O(n)

This is because the program sorts `2n` events and scans them once.

## Output
The program prints the year and population count for the year when the maximum number of people were alive.
