# Hu-Tucker Algorithm — Optimal Alphabetic Binary Tree

Given ordered weights `w1..wn`, build a binary tree whose leaves appear **left-to-right in index order** and
which minimizes `Σ wi · depth(i)`. (Equivalent to an optimal alphabetic code, or an optimal order-preserving merge pattern.)

## Files
- `hu_tucker.c` — implementation, validation and random tester.
- `README.md` — this file.

## Build & Run
```
gcc -O2 -Wall -o hu_tucker hu_tucker.c
./hu_tucker < input.txt          # solve one instance
./hu_tucker --test [trials]      # random validation against DP (default 2000)
```

## Input representation
Plain text: first `n`, then `n` non-negative integer weights in index order.
```
6
1 2 3 4 5 6
```
Internally the weights are an array in fixed positional order. For Phase 1 the working list is an array of
records `{id, weight, is_square}`: *square* = original leaf, *circle* = merged node. The array order is the
alphabetic order, so "no leaf between two nodes" is just a scan over adjacent array cells.

## Output
```
Leaf depths : 4 4 3 2 2 2
Tree        : ((((1 2) 3) 4) (5 6))
Hu-Tucker cost : 51
DP optimum     : 51  [MATCH]
In-order preserved & cost recomputed: OK (51)
```
Second example: input `8 / 10 1 1 10 1 1 10 1` → depths `3 4 4 2 3 3 3 3`, tree `(((1 (2 3)) 4) ((5 6) (7 8)))`, cost 97 (matches DP).
Third: five equal weights `5 5 5 5 5` → depths `3 3 2 2 2`, cost 60.

## Solution description
Plain Huffman merging of the two smallest weights breaks the order constraint. Hu–Tucker fixes this in three phases:

1. **Combination.** Two nodes are *compatible* if no original leaf (square) lies strictly between them
   (merged circles in between are fine; a square may itself be an endpoint). Repeatedly pick the compatible pair with the
   smallest weight sum (ties: leftmost `i`, then leftmost `j`), replace it by one circle of the summed weight placed at the left
   position. Repeat `n-1` times. This produces a tree that may *not* be order-preserving, but it is ordered enough to
   give the correct depths.
2. **Level assignment.** Read off the depth of each original leaf in the Phase 1 tree. Parents have larger ids than children, so one
   pass from the root downward computes all depths (no recursion).
3. **Recombination.** Scan leaves left to right with a stack of `(subtree, level)`. After pushing a leaf, while the top two
   levels are equal, merge them into one subtree at `level-1`. Using the Phase 2 depths this yields a tree with exactly
   those depths and leaves in index order — and therefore the same optimal cost.

**Finding the pair in O(n):** for each position `i`, `best[i]` is the lightest node among positions `i+1 …` up to and including
the first square after `i`. Scanning right to left, `best[i] = min(w[i+1], best[i+1])` unless `i+1` is a square (then it is just `i+1`).
The step's pair is `argmin_i w[i] + w[best[i]]`.

## Validation
`--test` runs random instances (n ≤ 40, weights including zeros and many duplicates) and checks that
(a) the Hu-Tucker cost equals the optimum from an independent O(n³) interval DP
`C[i][j] = min_k C[i][k]+C[k+1][j] + W(i,j)`, (b) the rebuilt tree has leaves in exact order 1..n, and
(c) its recomputed cost equals the claimed cost. Result: 20000 trials, 0 failures. Single runs also print a DP comparison when n ≤ 400.

## Complexity analysis
| Part | Time | Space |
|---|---|---|
| Phase 1: `n-1` steps × (O(n) best-partner scan + O(n) min + O(n) array delete) | **O(n²)** | O(n) |
| Phase 2: one pass over `2n-1` nodes | O(n) | O(n) |
| Phase 3: each leaf pushed once, each merge pops once | O(n) | O(n) |
| **Total** | **O(n²)** | **O(n)** |

The reference DP used only for validation is O(n³). 

The O(n²) bound is for this straightforward array implementation. The original Hu–Tucker paper's
priority-queue-based version, and the Garsia–Wachs formulation (Knuth 1973), reach **O(n log n)** time by keeping
compatible-pair minima in a heap / using a balanced search structure instead of rescanning. The lower bound
for the problem is Ω(n log n) (it contains sorting-like difficulty; Huffman coding is a special case).

Correctness depends on the Hu-Tucker theorem: the Phase 1 depths are optimal among all alphabetic trees, and Phase 3 can
always realize them in order. The code verifies this empirically on every test rather than assuming it.
