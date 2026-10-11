# Reorganise String with K-Distance Apart

Files: `reorganize.c` (solver + validation), `sample_input.txt`, `sample_input2.txt`, this README.

Build and run:
```
gcc -O2 -o reorganize reorganize.c
./reorganize < sample_input.txt
./reorganize --test     # validation
./reorganize --bench    # empirical timing
```

## 1. Problem
Given a string S and an integer K, rearrange the characters of S so that any two equal characters at positions i < j satisfy **j − i ≥ K**.
If this is impossible, return the empty string. For K ≤ 1 there is no restriction.

## 2. Input / output format
Input:
```
K
S
```
`S` is the whole second line (any bytes except newline). Output: the rearranged string in quotes, or `""  (impossible)`.

**Input representation.** The string is reduced to a frequency table `cnt[256]` over its d distinct characters, plus a cooldown table `ready[c]` = the earliest position at which character c may be used again. The order of the original characters is irrelevant, only the multiset matters.

### Example 1 (`sample_input.txt`)
```
2
aaadbbcc
```
Output:
```
K = 2, n = 8
Rearranged string: "abacabcd"
```
`a` is at positions 0, 2, 4; `b` at 1, 5; `c` at 3, 6, so every equal pair is at least 2 apart.

### Example 2 (`sample_input2.txt`) – impossible
```
3
aaabc
```
Output:
```
K = 3, n = 5
Rearranged string: ""  (impossible)
```
Three `a`s need positions at least 3 apart, so the span is at least 2·3 + 1 = 7 > 5.

## 3. Solution description

### Greedy: most remaining copies first, among the characters that are "ready"
```
cnt[c] = frequency;  ready[c] = 0 for all c
for pos = 0 .. n-1:
    choose c with cnt[c] > 0 and ready[c] <= pos that has the LARGEST cnt[c]
        (ties: smaller character, so the output is deterministic)
    if there is none: return "" (impossible)
    place c at pos;  cnt[c]--;  ready[c] = pos + K
```
The characters with the most copies left are the hardest to schedule, so they are used as soon as they are allowed. Characters that were just used are "cooling down" for K − 1 positions.

Two implementations are given, and they pick the same character at every step (the test checks that the outputs are identical):

* **scan**: each step scans the d distinct characters, O(n·d).
* **heap**: a max-heap (by remaining count) holds the ready characters and a FIFO queue holds the cooling ones (release times increase, so a plain queue is enough), O(n log d).

### Why it works
* **A necessary condition.** Let m be the maximum frequency and c the number of characters that have frequency m. The first copies of those c characters sit at distinct positions, so one of them starts at position ≥ c − 1 and its last copy is at position ≥ c − 1 + (m − 1)·K. This must be ≤ n − 1, hence feasibility requires **(m − 1)·K + c ≤ n** (for K ≥ 2).
* **The greedy attains it (exchange-argument sketch).** If some valid arrangement exists from the current state and it uses a ready character y while the greedy uses a ready character x with at least as many copies left, the later copies of x can be swapped with those of y at the corresponding positions without bringing equal characters closer than K. Using the highest-count character first never makes the remaining instance harder, so the greedy gets stuck only when no arrangement exists. I have not written this as a full formal proof; instead it is checked exhaustively on small cases (next section), and the formula above agrees with the greedy on every tested instance.

## 4. Validation (`./reorganize --test`)
1. **Brute force** (n ≤ 9, alphabet up to 4 letters, K = 0..6, 3000 random cases): a DFS tries every arrangement and says whether a valid one exists. The greedy (both versions) and the formula `(m−1)·K + c ≤ n` give the same yes/no answer in all 3000 cases, and each produced string is checked to be a permutation of the input with every equal pair at least K apart. 3000/3000 pass.
2. **Larger random inputs** (n ≤ 2000, alphabets of 1 to 256 symbols, K < 20): the scan and heap versions return identical strings, every output is valid, and feasibility equals the formula. 1000/1000 pass (521 feasible, 479 impossible, so both outcomes are exercised).
3. **Special cases:** `aabbcc` with K = 3 gives `abcabc`; K = 4 is impossible; `aaadbbcc` with K = 2 gives `abacabcd`; `aaabc` with K = 3 is impossible; K = 0 and K = 1; empty string; single character.

## 5. Complexity
Let n = |S|, d = number of distinct characters (d ≤ 256 for bytes).

| Part | Time | Space |
|---|---|---|
| Count frequencies | O(n) | O(d) |
| Scan version: n steps × scan of d characters | **O(n·d)** = O(n) for a fixed alphabet | O(d) + output |
| Heap version: n steps × heap ops of cost O(log d) | **O(n log d)** | O(d) + output |
| Feasibility formula (optional pre-check) | O(n) | O(d) |

* The output string needs O(n) space.
* Ω(n) is a lower bound (the output has n characters), so for a constant alphabet both versions are linear and optimal.
* For a large alphabet (for example when characters are replaced by integers or words) the heap version, O(n log d), is the better choice.

Measured (`--bench`):

* Growth in n (26 letters, K = 5): scan 0.011 s at n = 200 000 up to 0.317 s at 6.4 million; heap 0.005 s up to 0.163 s. Both double when n doubles, i.e. linear.
* Growth in d (n = 1 000 000): scan 0.0025 s (d = 2), 0.031 s (d = 16), 0.109 s (d = 64), 0.49 s (d = 256) shows the O(n·d) growth, while the heap stays between 0.005 s and 0.054 s. For tiny alphabets (d = 2 to 4) the scan version is slightly faster because it has no heap overhead.

## 6. Assumptions
* Characters are bytes (0–255). Positions are 0-based and "distance" is the difference of positions, so K = 1 means no restriction and K = 2 means at least one other character between equal ones.
* If several arrangements exist, the program returns the one produced by the tie-break "largest remaining count, then smaller character"; any valid arrangement is acceptable.
