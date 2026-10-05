# Q04 - Longest Increasing Subsequence (LIS)

## Question Title

Given an integer array `A = [a₀, a₁, ..., aₙ₋₁]`, find the length of the longest subsequence such that all elements of the subsequence are strictly increasing.

## How the Code Works

The user chooses manual or random input. `computeLIS()` fills a 1-D table `dp[i]` (LIS length ending at index `i`) using a nested loop that checks all prior elements. A `parent[]` array tracks which predecessor gives the optimal LIS at each position. After the table is filled, `printLIS()` follows parent pointers from the index with the maximum `dp` value back to the start, reverses the path, and prints the actual subsequence.

## Maths / Logic Behind This

Let `dp[i]` = length of the strictly increasing subsequence that ends at index `i`.

**Recurrence:**
```
dp[i] = 1   (base: subsequence containing only A[i])

dp[i] = max{ dp[j] + 1 : j < i  and  A[j] < A[i] }
```

**Why this is correct:**
For each index `i`, we look backward at every `j < i`. If `A[j] < A[i]`, then `A[i]` can extend any increasing subsequence ending at `j`. We choose the longest such extension. This satisfies optimal substructure because the best LIS ending at `i` is built directly from the best LIS ending at some earlier position.

The overall answer is `max{ dp[i] }` over all `i`.

## Complexity Analysis

- **Time complexity:** O(n²) — two nested loops, each up to n
- **Extra space complexity:** O(n) — for `dp[]` and `parent[]` arrays