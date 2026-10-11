# Minimise Deviation in Array (Two-Way Greedy with Heaps)

Files: `deviation.c` (solver + validation), `sample_input.txt`, `sample_input2.txt`, this README.

Build and run:
```
gcc -O2 -o deviation deviation.c
./deviation < sample_input.txt
./deviation --test     # validation
./deviation --bench    # empirical timing
```

## 1. Problem
Given positive integers a_1..a_n, one operation either doubles an **odd** element or halves an **even** element (any number of times, on any elements). The deviation is max(A) − min(A). Minimise it.

## 2. Input / output format
Input:
```
n
a_1 a_2 ... a_n
```
Output: the minimum deviation and, for every element, its original and final value (a final array that achieves the minimum).

**Input representation (normal form).** Write a_i = o_i · 2^e_i with o_i odd. The values reachable from a_i are exactly

    { o_i · 2^k : 0 ≤ k ≤ kmax_i },   kmax_i = max(e_i, 1)

An odd number can be doubled once (to 2·o_i) and halved back; an even number can be halved down to o_i, but an even number can never be doubled, so it can never exceed its original value. Each element is therefore a *chain of candidate values*, stored as the pair (o_i, kmax_i), and the problem becomes: **choose one value from every chain so that max − min is smallest.** (The program checks this model against a BFS on the real operations.)

### Example 1 (`sample_input.txt`)
```
5
4 1 5 20 3
```
Output:
```
Minimum deviation = 3
index    original     final
1        4            4
2        1            2
3        5            5
4        20           5
5        3            3
```
The final array 4, 2, 5, 5, 3 has max 5 and min 2.

### Example 2 (`sample_input2.txt`)
```
4
1 2 3 4
```
Output:
```
Minimum deviation = 1
index    original     final
1        1            2
2        2            2
3        3            3
4        4            2
```

## 3. Solution description

### Down-greedy with a max-heap (main algorithm)
1. Raise every element to the top of its chain: o_i · 2^kmax_i (odd numbers are doubled once, even numbers stay). Now every element is at its largest value and the only useful operation is halving.
2. Keep a max-heap of the current values and the current minimum `mn`. Record the best deviation `top − mn`.
3. Repeat: pop the maximum; if it is already at its smallest value o_i, stop; otherwise halve it (one step down its chain), push it back, update `mn`, and update the best deviation.

### Up-greedy with a min-heap (second implementation, used as a cross-check)
Start every element at o_i (all odd), keep the current maximum, and repeatedly raise the **minimum** element by one step (×2) until it reaches its cap o_i · 2^kmax_i. This is the mirror image: the two directions use the two operations in opposite roles ("two-way").

### Reconstructing the array
If the best deviation was seen when the maximum was `H`, every element takes the **largest candidate that is ≤ H**.

### Why the down-greedy is optimal
Let an optimal choice be b_1..b_n with maximum H\* and deviation D\*. The greedy starts with every value ≥ b_i (it is at the top of the chain).
* While the current maximum M is greater than H\*, the element being lowered has value M > H\* ≥ b_i, so b_i lies strictly below it in the chain: it can be lowered again, the greedy does not stop, and after lowering it is still ≥ b_i.
* The maximum never increases, so there is a first moment when M ≤ H\*. At that moment all current values are still ≥ b_i ≥ H\* − D\*, so the deviation is ≤ H\* − (H\* − D\*) = D\*.
* The greedy only ever looks at real assignments, so its best value is also ≥ D\*. Hence it returns D\*. The up-greedy is symmetric.

## 4. Validation (`./deviation --test`)
1. **BFS on the real operations** (n ≤ 4, values ≤ 24, 1500 random cases): all arrays reachable by "odd → ×2, even → ÷2" are enumerated; the minimum deviation over them is the ground truth. It matches the down-greedy, the up-greedy, an exhaustive enumeration of one candidate per chain, and the sorted-window baseline. The BFS also asserts that every reachable value really is o_i · 2^k with k ≤ kmax_i. 1500/1500 pass.
2. **Larger inputs** (n ≤ 300, values up to 10^9, up to 10^4, up to 64, and arrays of powers of two): down-greedy = up-greedy = **window baseline** (sort all candidates, find the smallest range that contains one candidate of every element, with two pointers). The final array is checked: every value is reachable from the original element and its deviation equals the reported minimum. 1000/1000 pass.
3. **Special cases:** `[1,2,3,4]` → 1, `[4,1,5,20,3]` → 3, `[2,10,8]` → 3, one element → 0, equal elements → 0, and `[2^30, 1]`.

## 5. Complexity
Let A be the maximum value and L = ⌊log₂ A⌋ + 1. Every element has at most L + 1 candidates, so the total number of candidates is N ≤ n·(L + 1).

| Part | Time | Space |
|---|---|---|
| Normalise (strip factors of 2) | O(n·L) in the worst case, O(n) amortised | O(n) |
| Build the heap | O(n) | O(n) |
| Greedy loop: at most N heap pops/pushes, each O(log n) | **O(n·L·log n)** | O(n) |
| Window baseline (validation only) | O(N log N) | O(N) |

* The loop stops after at most Σ kmax_i ≤ n·L steps, because every step lowers (or raises) one element by one position in its chain.
* A single run needs only O(n) extra memory; the candidates are never stored, only (o_i, kmax_i) and the current level.
* Both heap versions have the same bound; the up-version is usually much faster in practice because it stops as soon as the smallest element reaches its cap.

Measured (`--bench`): random values up to 10^9, n = 50 000 → 0.002 s, 100 000 → 0.004 s, 200 000 → 0.009 s, 400 000 → 0.039 s, 800 000 → 0.09 s for the down version (the up version needs 0.001 to 0.012 s). These random inputs usually have a large odd part, so the loops end early. For an adversarial input with many halvings per element (n = 200 000, small odd parts times a power of two) the time grows with the value range: 0.013 s (up to 2^5), 0.063 s (2^18), 0.20 s (2^31), which shows the extra log A factor.

## 6. Assumptions
* Elements are positive integers; the largest value after the initial doubling must fit in a signed 64-bit integer (inputs up to about 4·10^18 are fine).
* Operations may be applied any number of times in any order; the final array may leave any element unchanged.
