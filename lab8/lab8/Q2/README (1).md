# Q02 - Coin Change: Total Number of Combinations

## Question Title

Given an array of distinct positive integers representing coin denominations `C = {c₁, c₂, ..., cₙ}` and a target amount `V`, find the total number of distinct combinations of coins that sum up to `V`. You may assume an infinite supply of each coin denomination. The order of coins does not matter (e.g., `1 + 2` and `2 + 1` are considered the same combination).

## How the Code Works

The user picks manual or random input. The `countWays()` function initialises `dp[0] = 1` and `dp[v] = 0` for `v > 0`, then iterates coins in the **outer** loop and amounts in the **inner** loop. This outer-coin ordering is critical: it ensures that each distinct multiset is counted exactly once, regardless of the order its coins are used. The final answer is returned in `dp[V]`.

## Maths / Logic Behind This

Define `dp[v]` = number of distinct combinations summing to `v`.

**Recurrence (coins outer, amounts inner):**
```
dp[0] = 1
For each coin cᵢ:
    For v = cᵢ to V:
        dp[v] += dp[v - cᵢ]
```

**Why outer-coin ordering prevents permutation duplicates:**
By fixing the coin ordering in the outer loop, when we compute `dp[v]` using coin `cᵢ`, all previously added contributions use only coins `c₁, c₂, ..., cᵢ`. Thus `{1,2}` and `{2,1}` are never counted separately — they map to the same state transition.

This is equivalent to counting partitions of `V` into parts from the coin set, a classic combinatorics problem solved optimally by DP.

## Complexity Analysis

- **Time complexity:** O(n × V) — n coins × V amount states
- **Extra space complexity:** O(V) — single 1-D DP array of size V+1