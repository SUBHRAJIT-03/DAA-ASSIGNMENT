# Q07 - Rod Cutting with Reconstruction

## Question Title

Given a rod of length `n` inches and an array of prices `P = [p₁, p₂, ..., pₙ]`, where `pᵢ` denotes the market price of a rod piece of length `i` inches, determine:
1. The maximum revenue obtainable by cutting up the rod and selling the pieces.
2. The exact lengths of the pieces that constitute the optimal decomposition (reconstruction).

Cuts are integral and can be made in any combination (including leaving the rod uncut), and the sum of the piece lengths must equal `n`.

## How the Code Works

The user provides the rod length and a 1-indexed price table (manual or random). `rodCutting()` builds a 1-D DP table `dp[0..n]` where `dp[l]` is the maximum revenue for a rod of length `l`. A parallel `cut[l]` array records which first-cut length achieves that optimum at each length. After the table is filled, `printCuts()` follows the `cut[]` array — subtracting the first-cut length from the remaining rod — until the remainder is zero.

## Maths / Logic Behind This

Let `dp[l]` = maximum revenue achievable from a rod of length `l`.

**Recurrence:**
```
dp[0] = 0

dp[l] = max{ P[k] + dp[l-k] : 1 ≤ k ≤ l }
```

At each rod length `l`, we try every possible first-cut of length `k` (from 1 to `l`). The piece of length `k` sells at price `P[k]`, and the remaining rod of length `l-k` is solved optimally by `dp[l-k]` (optimal substructure). The best first cut is stored in `cut[l]` for reconstruction.

**Reconstruction:** Starting from `n`, repeatedly apply `n ← n - cut[n]` until `n = 0`, printing each `cut[n]` along the way.

## Complexity Analysis

- **Time complexity:** O(n²) — for each of n lengths we check up to n first-cut choices
- **Extra space complexity:** O(n) — for `dp[]` and `cut[]` arrays