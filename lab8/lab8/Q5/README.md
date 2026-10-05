# Q05 - Maximum Sum Increasing Subsequence (MSIS)

## Question Title

Given an array of `n` positive integers `A = [a₀, a₁, ..., aₙ₋₁]`, find the maximum possible sum of a strictly increasing subsequence.

## How the Code Works

The user selects manual or random (positive integers) input. `computeMSIS()` initialises `dp[i] = A[i]` for all `i` (each element alone is a valid subsequence), then for each `i` checks all prior indices `j` where `A[j] < A[i]`: if `dp[j] + A[i]` beats `dp[i]`, it updates both `dp[i]` and `parent[i]`. The index with the highest `dp` value is recorded for traceback. `printMSIS()` reconstructs the actual subsequence using the parent chain.

## Maths / Logic Behind This

Let `dp[i]` = maximum sum of a strictly increasing subsequence ending at index `i`.

**Recurrence:**
```
dp[i] = A[i]   (base: only element A[i])

dp[i] = max{ dp[j] + A[i] : j < i  and  A[j] < A[i] }

Answer = max{ dp[i] }  for all i
```

**Comparison with LIS:** This is structurally identical to the O(n²) LIS algorithm, except instead of tracking *count* (`dp[j] + 1`) we track *sum* (`dp[j] + A[i]`). The optimal substructure is preserved: the best sum ending at `i` depends only on the best sum ending at the optimal predecessor `j`.

**Why positivity matters:** Because all values are positive, extending a subsequence always increases the sum, so the greedy intuition of "always extend" can be formalised through the DP.

## Complexity Analysis

- **Time complexity:** O(n²) — two nested loops over n elements
- **Extra space complexity:** O(n) — for `dp[]` (long long) and `parent[]` arrays