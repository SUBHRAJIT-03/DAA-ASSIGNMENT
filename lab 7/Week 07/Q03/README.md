## Q03 - Reve's Puzzle (4 Pegs)

## How the Code Works
The user chooses either manual input or random generation for the number of disks `n`. The program then computes the minimum number of moves required to move all disks from one peg to another using four pegs instead of three.

The solution uses dynamic programming:
- `T3[i]` stores the minimum moves for the classic 3-peg Tower of Hanoi problem.
- `T4[i]` stores the minimum moves for the 4-peg version.

For each `i`, the program tests all possible split points `k` and chooses the best value using the Frame-Stewart recurrence.

## Maths / Logic Behind This
For 3 pegs, the classic formula is:

T3(n) = 2^n - 1

For 4 pegs, the recurrence is:

T4(n) = min(2 × T4(k) + T3(n - k))

for `1 <= k < n`

This means we move the top `k` disks to an auxiliary peg, move the remaining `n-k` disks with the 3-peg strategy, and then move the `k` disks again. The best split point is selected to minimize the total moves.

## Complexity Analysis
- Time Complexity: O(n²)
- Space Complexity: O(n)

This is because the program builds a DP table up to `n` and checks all valid split points for each value.

## Output
The program prints:
- the selected value of `n`
- the minimum number of moves to solve the 4-peg puzzle
