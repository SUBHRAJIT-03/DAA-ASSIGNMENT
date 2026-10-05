# Q01 - Minimum Coin Change

## Question Title

Given an integer array of coin denominations `C = {c₁, c₂, ..., cₙ}` representing coins of different values, and an integer target amount `V`, find the minimum number of coins needed to make up that amount. You may assume an infinite supply of each coin denomination. If that amount of money cannot be made up by any combination of the coins, return `−1`.

## How the Code Works

At startup the user selects **manual** or **random** input. The core `minCoins()` function fills a 1-D DP table `dp[0..V]` bottom-up, where `dp[v]` stores the minimum coins for amount `v`. A parallel `parent[]` array records which coin denomination was chosen at each step, enabling traceback. The final answer is `dp[V]`, or `−1` if it remains `INT_MAX`.

## Maths / Logic Behind This

Define `dp[v]` = minimum number of coins to make amount `v`.

**Recurrence:**
```
dp[0] = 0
dp[v] = min{ dp[v - cᵢ] + 1 }  for all cᵢ ≤ v,  1 ≤ i ≤ n
dp[v] = ∞   if no coin satisfies cᵢ ≤ v
```

Each sub-problem `dp[v]` is solved once using previously computed smaller sub-problems (optimal substructure). The infinite supply of each coin makes this an **unbounded** variant — the same denomination can be reused freely.

**Traceback:** `parent[v] = cᵢ` records which coin reduced the amount, so the path can be reconstructed by following `v → v − parent[v] → ...` until `0`.

## Complexity Analysis

- **Time complexity:** O(n × V) — for each of V amounts we iterate over n denominations
- **Extra space complexity:** O(V) — for the `dp[]` and `parent[]` arrays (both of size V+1)
