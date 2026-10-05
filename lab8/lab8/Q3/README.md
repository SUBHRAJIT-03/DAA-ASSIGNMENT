# Q03 - Longest Common Subsequence (LCS)

## Question Title

Given two sequences `X = <x₁, x₂, ..., xₘ>` and `Y = <y₁, y₂, ..., yₙ>`, compute the length of their longest common subsequence and reconstruct the actual subsequence string.

## How the Code Works

The user selects manual or random (lowercase letter) input. `computeLCS()` fills a 2-D DP table `dp[0..m][0..n]` bottom-up and simultaneously records movement directions in `dir[][]` (MATCH / DELETE / INSERT). Once the table is complete, `traceback()` starts at `dp[m][n]` and follows the direction pointers back to `dp[0][0]`, collecting matched characters in reverse, then reverses the collected string to obtain the final LCS.

## Maths / Logic Behind This

Let `L(i, j)` = length of LCS of `X[1..i]` and `Y[1..j]`.

**Recurrence:**
```
L(i, 0) = 0,  L(0, j) = 0

         ┌ L(i-1, j-1) + 1        if X[i] = Y[j]
L(i,j) = ┤ max(L(i-1,j), L(i,j-1)) otherwise
```

The optimal substructure follows from two cases:
1. **Characters match** — the LCS must include this character, reducing to `L(i-1, j-1)`.
2. **No match** — the LCS is found in either `X[1..i-1]` & `Y[1..j]`, or `X[1..i]` & `Y[1..j-1]`.

Traceback reconstructs the LCS by following MATCH steps diagonally through `dir[][]`.

## Complexity Analysis

- **Time complexity:** O(m × n) — fill each cell of the m×n DP table once
- **Extra space complexity:** O(m × n) — for `dp[][]` and `dir[][]` tables; reducible to O(min(m,n)) if only the length is needed