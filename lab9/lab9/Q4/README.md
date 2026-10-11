# Minimum Cost to Connect Sticks

Files: `sticks.c` (solver + validation), `sample_input.txt`, `sample_input2.txt`, this README.

Build and run:
```
gcc -O2 -o sticks sticks.c
./sticks < sample_input.txt
./sticks --test     # validation
./sticks --bench    # empirical timing
```

## 1. Problem
Given sticks of lengths L_1..L_n, joining two sticks of lengths x and y costs x + y and produces one stick of length x + y.
Find the minimum total cost to end with a single stick.

## 2. Input / output format
Input:
```
n
L_1 L_2 ... L_n
```
Output: the minimum total cost and the merges in the order they are performed.
For n = 0 or n = 1 the cost is 0.

**Input representation.** The lengths are sorted once into an array (queue Q1). Every stick created by a merge is appended to a second array (queue Q2). Merged lengths are produced in non-decreasing order, so Q2 is always sorted too, and the two smallest sticks are always at the fronts of Q1 and Q2. No priority queue is needed.

### Example 1 (`sample_input.txt`)
```
3
2 4 3
```
Output:
```
Minimum total cost = 14
  merge 2 + 3 -> 5 (cost 5)
  merge 4 + 5 -> 9 (cost 9)
```

### Example 2 (`sample_input2.txt`)
```
6
1 2 3 4 5 6
```
Output:
```
Minimum total cost = 51
  merge 1 + 2 -> 3 (cost 3)
  merge 3 + 3 -> 6 (cost 6)
  merge 4 + 5 -> 9 (cost 9)
  merge 6 + 6 -> 12 (cost 12)
  merge 9 + 12 -> 21 (cost 21)
```

## 3. Solution description

### Greedy: always join the two shortest sticks
```
sort lengths -> Q1;  Q2 = empty;  cost = 0
repeat n-1 times:
    x = smallest front of Q1/Q2 (remove it)
    y = next smallest front of Q1/Q2 (remove it)
    cost += x + y;  append x + y to the end of Q2
```
On ties the leaf (Q1) is taken first.

### Why it is optimal
1. **Cost = Σ L_i · depth_i.** A sequence of merges is a binary tree whose leaves are the original sticks. A stick of length L_i is counted once in every merge on the path from its leaf to the root, so the total cost is the sum of L_i times its depth. Minimising the cost is exactly the problem of building a binary tree with minimum weighted leaf depth, i.e. a **Huffman tree** (the stick lengths play the role of symbol frequencies).
2. **Exchange argument.** In some optimal tree the two shortest sticks a, b are siblings at maximum depth: if a longer stick were deeper, swapping it with a would not increase Σ L·depth. So we may merge a and b first.
3. **Induction.** After merging a and b into one stick of length a + b, the cost of the rest equals the optimal cost of the problem with n − 1 sticks, and the merge itself costs a + b. By induction the greedy is optimal.

### Why Q2 stays sorted
If the merge sums are s_1, s_2, ..., then s_{t+1} ≥ s_t: the two smallest sticks after step t are each at least as large as the two smallest before it. So appending to the end keeps Q2 sorted, and the minimum of the whole collection is always at one of the two queue fronts.

## 4. Validation (`./sticks --test`)
1. **Brute force** (n ≤ 8, with ties and zero lengths): recursively try *every* pair to merge at every step and take the minimum. 600/600 pass.
2. **Independent baselines** (n ≤ 500): an O(n²) version (scan for the two minima) and a binary min-heap version give the same cost. 500/500 pass.
3. **Structural checks on every test:**
   * the cost equals Σ L_i · depth_i of the merge tree (the Huffman identity above),
   * the printed merges are replayed on a multiset of the original lengths: every merged pair exists, the final single stick equals the total length, and the summed costs equal the reported cost.
4. **Special cases:** n = 0 (cost 0), n = 1 (cost 0), 8 equal sticks (cost 24 = 8 · 3), and Fibonacci lengths (a worst case where the merge tree is a chain).

## 5. Complexity

| Part | Time | Space |
|---|---|---|
| Sort lengths | O(n log n) | O(n) |
| Two-queue merging: n − 1 steps, O(1) each | **O(n)** | O(n) |
| **Total** | **O(n log n)** | **O(n)** |

* With a binary min-heap each of the n − 1 steps does two extractions and one insertion, O(log n) each, so the merging alone is O(n log n); the total is the same O(n log n), but the two-queue version has a smaller constant.
* If the input is already sorted (or the lengths are small integers and can be counting-sorted) the whole algorithm is O(n).
* The O(n²) method that scans for the two minima is the simple baseline.
* Lower bound: the problem is as hard as Huffman coding, which has an Ω(n log n) comparison lower bound for unsorted input.

Measured (`--bench`, random lengths up to 10⁶): n = 100 000 → 0.011 s, 200 000 → 0.023 s, 400 000 → 0.048 s, 800 000 → 0.157 s, 1 600 000 → 0.326 s for the two-queue method, and the heap version is about 20–50 % slower (0.016 s to 0.398 s) with the same cost.

## 6. Assumptions
* Lengths are non-negative integers. The cost can reach about n · (sum of lengths), so 64-bit integers are used; inputs must keep that value below 2^63.
* Several sticks of equal length are treated as distinct sticks.
