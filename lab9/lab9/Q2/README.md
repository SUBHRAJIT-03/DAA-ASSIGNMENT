# Huffman Coding with Canonical Codebook

Files: `huffman.c` (solver + validation), `sample_input.txt`, `sample_input2.txt`, this README.

Build and run:
```
gcc -O2 -o huffman huffman.c -lm
./huffman < sample_input.txt
./huffman --test     # validation
./huffman --bench    # empirical timing
```

## 1. Problem
Given symbols with frequencies f_i, find a prefix-free binary code minimising the expected length Σ f_i·l_i / Σ f_i
(equivalently the total number of bits Σ f_i·l_i), and print the **canonical** codebook: symbols ordered by (code length, symbol), with codes assigned in increasing numeric order.

## 2. Input / output format
Input:
```
n
name_1 freq_1
...
name_n freq_n
```
Output: the codebook in canonical order (symbol, frequency, length, code), the total number of encoded bits, the expected length and (for reference) the entropy.
Symbols with frequency 0 get no code; a single symbol gets the code `0`.

**Input representation.** The symbols are kept as a flat array of (name, frequency). A sorted array of leaf weights plus a second queue of internal nodes replaces the priority queue (see below). The tree is stored only as a `parent[]` array over the 2m−1 nodes, because the canonical codebook depends only on the code *lengths*.

### Example 1 (`sample_input.txt`)
```
6
a 45
b 13
c 12
d 16
e 9
f 5
```
Output:
```
symbol       freq       len    code
a            45         1      0
b            13         3      100
c            12         3      101
d            16         3      110
e            9          4      1110
f            5          4      1111
Total encoded bits = 224
Expected length = 2.240000 bits/symbol  (entropy = 2.219880)
```

### Example 2 (`sample_input2.txt`, Fibonacci frequencies, one zero-frequency symbol)
```
Output:
H   21  1  0
G   13  2  10
F   8   3  110
E   5   4  1110
D   3   5  11110
C   2   6  111110
A   1   7  1111110
B   1   7  1111111
Total encoded bits = 132, expected length 2.444444 (entropy 2.371389)
```
`Z` (frequency 0) receives no code.

## 3. Solution description

### Step 1 – Code lengths (Huffman's greedy algorithm, two-queue form)
1. Sort the m non-zero-frequency symbols by frequency (ties by input position).
2. Keep two queues: Q1 = the sorted leaves, Q2 = internal nodes created so far.
3. Repeat m−1 times: remove the two smallest weights from the fronts of Q1/Q2 (on ties take the leaf), create a parent whose weight is their sum, and append it to Q2.
4. Because merged weights are produced in non-decreasing order, Q2 is automatically sorted, so no heap is required.
5. Parents always have a larger index than their children, so depths are filled in one backward pass; the depth of a leaf is its code length.

*Why it is optimal* (standard exchange argument): in some optimal tree the two least frequent symbols are siblings at maximum depth. Merging them into one symbol of weight f_a + f_b reduces the problem to m−1 symbols, and the cost differs by exactly f_a + f_b. Induction gives optimality.

### Step 2 – Canonical codebook from the lengths
Order the symbols by (length, name). Give the first symbol the all-zero code of its length; each next code is `(previous + 1)` followed by as many zeros as the length increases. The codes are stored as bit strings, so depths larger than 64 (for example Fibonacci-like inputs) cause no overflow. Because the lengths satisfy the Kraft equality Σ 2^(−l_i) = 1, this assignment is prefix-free.

Different Huffman trees may have different length multisets (ties), but all have the same minimum total cost. The tie rule "prefer a leaf" gives a deterministic result, and it also yields the minimum-variance code.

## 4. Validation (`./huffman --test`)
1. **Brute force** (2 ≤ n ≤ 8, many ties): the minimum of Σ f_i·l_i over *all* length vectors that satisfy Kraft with equality (that is, all full binary trees) equals the Huffman cost. 400/400 pass.
2. **Independent baseline**: a textbook O(n²) Huffman (repeatedly extract the two minima, cost = sum of merged weights) gives the same cost for random inputs up to n = 200, including zero frequencies and heavy ties. 300/300 pass.
3. **Structural checks on every test**: pairwise prefix-freeness, Kraft sum = 1, and the canonical property (each code equals the previous code + 1, shifted by the length gap).
4. **Worst case**: Fibonacci frequencies with n = 80, giving depth n−1 = 79, pass.

## 5. Complexity

| Part | Time | Space |
|---|---|---|
| Filter non-zero symbols | O(n) | O(n) |
| Sort by frequency | O(n log n) | O(n) |
| Two-queue merging (m−1 steps, O(1) each) | **O(n)** | O(n) |
| Depth computation | O(n) | O(n) |
| Canonical ordering by (length, name) with `qsort` | O(n log n) | O(n) |
| Generating codes (total output length Σ l_i ≤ n·L) | O(n·L), L = longest code ≤ n−1 | O(L) working buffer |
| **Total** | **O(n log n)** (+ output size) | **O(n)** |

* The merge itself is linear; the sort dominates. With a binary heap instead of the two-queue trick the merge would be O(n log n) as well, so the total is the same.
* If the frequencies are already sorted (or are small integers, counting sort), the tree construction costs O(n); counting sort by length also makes the canonical ordering O(n).
* The output (the codes themselves) needs Σ l_i characters, which is O(n log n) for typical inputs and O(n²) in the worst skewed case.

Measured (`--bench`, building the lengths for random frequencies): n = 100 000 → 0.031 s, 200 000 → 0.060 s, 400 000 → 0.108 s, 800 000 → 0.237 s, 1 600 000 → 0.473 s, i.e. a little more than ×2 per doubling, consistent with n log n.

## 6. Assumptions
* Frequencies are non-negative integers, and the sum of all frequencies fits in a signed 64-bit integer.
* Canonical tie-break: symbols with the same code length are ordered by `strcmp` on their names.
