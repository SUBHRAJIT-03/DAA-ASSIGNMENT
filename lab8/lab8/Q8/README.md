# Q08 - Optimal Binary Search Trees (OBST)

## Question Title

Given a set of `n` distinct sorted keys `K = <k₁, k₂, ..., kₙ>` with search probabilities `p₁, p₂, ..., pₙ`, and `n+1` dummy keys `d₀, d₁, ..., dₙ` representing unsuccessful searches with probabilities `q₀, q₁, ..., qₙ`, find the minimum expected search cost of a binary search tree.

## How the Code Works

The user provides (or the program randomly generates) keys, key probabilities, and dummy probabilities, which are then normalised to sum to 1. `computeOBST()` implements the CLRS Chapter 15 algorithm filling three tables — `e[i][j]` (expected cost), `w[i][j]` (weight/probability sum), and `root[i][j]` (optimal root index) — in order of increasing chain length `l`. Once the tables are complete, `printTree()` recursively reconstructs and prints the tree structure using `root[][]`, and `displayTables()` shows the full DP matrices.

## Maths / Logic Behind This

Let the keys be `k₁..kₙ` and dummies be `d₀..dₙ`. Define:

```
w[i][j] = q[i] + Σ (p[r] + q[r])  for r = i+1 to j

e[i][i] = q[i]   (base: only dummy key dᵢ)

e[i][j] = min{ e[i][r-1] + e[r][j] + w[i][j] }
           for r = i+1 to j
```

`w[i][j]` is added to the cost because every search through the subtree `kᵢ₊₁..kⱼ` incurs one extra comparison — the contribution of all probabilities in that subtree increases by 1 depth level when the subtree is placed under a new root.

`root[i][j]` stores the `r` that minimises `e[i][j]` and is used for tree reconstruction.

## Complexity Analysis

- **Time complexity:** O(n³) — three nested loops (chain length × left boundary × root choice); Knuth's monotone root optimisation reduces this to O(n²)
- **Extra space complexity:** O(n²) — for the `e[][]`, `w[][]`, and `root[][]` tables