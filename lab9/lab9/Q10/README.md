# Q10 - Greedy shortest-superstring algorithm versus the exact optimum

## Question Title
**[Greedy Superstring Conjecture]** (Open Unsolved Problem): Given a set of n strings S = {s₁, s₂, …, s_n}, find the shortest single string that contains every s_i as a substring. You are advised to explore more on this problem to develop an insight.

*Background from the handout:* the conjecture, formulated nearly four decades ago, hypothesised that the classic greedy algorithm (repeatedly merging the pair with the maximum overlap) always yields a superstring no more than twice the length of the optimal solution (2-approximation). A September 2026 arXiv paper titled "Disproving the Greedy Superstring Conjecture" reportedly shows that, for instances whose strings have even length k ≥ 10, the approximation ratio is at least (9k+2)/(4k+4), approaching 9/4 = 2.25 as k → ∞, which would disprove the 2-approximation claim. The handout notes that this claim has not been officially validated yet.

## How the Code Works
The program reads up to 12 strings (random mode: 3–8 strings over {a,b} or {a,b,c}, lengths 2–6), removes duplicates and strings contained in others, and builds the pairwise overlap table. `greedy_superstring()` repeatedly merges the pair with the largest overlap (ties: lowest indices), updating overlaps in O(1) per pair. `optimal_superstring()` computes the exact optimum with a bitmask DP over string orderings. It then prints both superstrings, the ratio |greedy| / |optimal| and checks that both outputs contain every input string.

## Maths / Logic Behind This
After removing strings that are substrings of others, every superstring corresponds to an ordering of the strings, and consecutive strings are merged using their overlap `ov(a, b)` = the longest suffix of a that is a prefix of b. Hence

|superstring| = Σ |s_i| − Σ ov(consecutive pairs),

so the shortest superstring is the maximum-overlap Hamiltonian path in the overlap graph (a TSP-like, NP-hard problem). The exact optimum used here is Held–Karp: `dp[mask][last]` = maximum total overlap using the strings in `mask` ending with `last`.

The greedy algorithm repeatedly takes the heaviest remaining overlap edge. Experiments with this program measure the ratio |greedy| / |optimal| on instances you provide; ratios up to 2 are consistent with the old conjecture, whereas a verified ratio above 2 would refute it. This program does **not** reproduce or verify the construction mentioned in the handout, and since that claim is unvalidated it should be treated as a claim, not a theorem.

## Complexity Analysis
- **Time complexity:** Greedy: O(n²L² + n³), where L is the maximum string length (overlap table in O(n²L²), then n − 1 merges each scanning the O(n²) overlap table). The exact optimum used for comparison costs O(2ⁿ·n²).
- **Extra space complexity:** O(n² + nL) for the greedy algorithm (overlap table and merged strings); O(2ⁿ·n) for the exact Held–Karp table.
