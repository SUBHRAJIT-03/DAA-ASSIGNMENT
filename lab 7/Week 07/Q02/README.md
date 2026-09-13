## Q02 - Super Egg Dropping

## How the Code Works
The program first asks whether the user wants to enter the number of eggs `E` and floors `F` manually or generate random values. It then builds a dynamic programming table where `dp[i][j]` stores the minimum number of trials required to determine the highest safe floor using `i` eggs and `j` floors.

For every possible floor `x`, the program tries the drop decision and considers the worst-case outcome:
- If the egg breaks, it tries the lower floors using `i - 1` eggs.
- If the egg survives, it tries the upper floors using `i` eggs.

The best result among all possible floor choices is stored as the answer.

## Maths / Logic Behind This
The standard recurrence is:

dp[i][j] = 1 + min over x of max(dp[i - 1][x - 1], dp[i][j - x])

This means:
- choose a floor `x` to test,
- add 1 trial,
- and take the maximum of the two worst-case branches.

Base cases:
- dp[0][j] = 0
- dp[1][j] = j
- dp[i][0] = 0
- dp[i][1] = 1

The algorithm computes the smallest number of trials that guarantees a solution in the worst case.

## Complexity Analysis
- Time Complexity: O(E × F × F) = O(E × F²)
- Space Complexity: O(E × F)

This is because the DP table stores states for each egg-floor pair and loops across all candidate floors.

## Output
The program prints the minimum number of trials needed to guarantee the highest safe floor is found for the chosen `E` and `F` values.
