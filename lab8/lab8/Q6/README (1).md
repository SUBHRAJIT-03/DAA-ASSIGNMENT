# Q06 - Edit Distance with Traceback Information

## Question Title

Given two strings `A` of length `m` and `B` of length `n`, compute the minimum number of operations (insertions, deletions, or substitutions) required to transform `A` into `B`, and print the traceback result showing each operation step.

## How the Code Works

The user selects manual or random string input. `computeEditDistance()` builds a 2-D table `dp[0..m][0..n]` using the Wagner-Fischer algorithm, simultaneously filling a direction matrix `dir[][]` that records which operation (MATCH, SUBSTITUTE, DELETE, INSERT) was chosen at each cell. `traceback()` starts at `dp[m][n]` and follows `dir[][]` backwards to `dp[0][0]`, collecting operations in reverse, then prints them in forward order with clear labels (KEEP / REPLACE / DELETE / INSERT).

## Maths / Logic Behind This

Let `dp[i][j]` = minimum edit distance between `A[1..i]` and `B[1..j]`.

**Recurrence (Wagner-Fischer):**
```
dp[i][0] = i    (delete all i chars of A)
dp[0][j] = j    (insert all j chars of B)

         ┌ dp[i-1][j-1]          if A[i] = B[j]   (match — free)
dp[i][j]=┤
         └ 1 + min(
               dp[i-1][j-1],     (substitute A[i] → B[j])
               dp[i-1][j],       (delete A[i])
               dp[i][j-1]        (insert B[j])
             )
```

**Traceback:** Each cell's `dir[i][j]` stores which of the three cases was chosen (or MATCH). Following pointers from `(m,n)` to `(0,0)` reconstructs the full sequence of edit operations.

## Complexity Analysis

- **Time complexity:** O(m × n) — fill each cell of the (m+1) × (n+1) DP table once
- **Extra space complexity:** O(m × n) — for `dp[][]` and `dir[][]`; reducible to O(min(m,n)) if only the distance is needed