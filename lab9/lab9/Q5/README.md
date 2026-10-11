# Candy Distribution (Bi-directional Slope Greedy)

Files: `candy.c` (solver + validation), `sample_input.txt`, `sample_input2.txt`, this README.

Build and run:
```
gcc -O2 -o candy candy.c
./candy < sample_input.txt
./candy --test     # validation
./candy --bench    # empirical timing
```

## 1. Problem
n children stand in a line with ratings r_1..r_n. Every child gets at least 1 candy, and a child with a **strictly higher** rating than an immediate neighbour must get **strictly more** candies than that neighbour. Find the minimum total number of candies.
(Equal ratings impose no constraint.)

## 2. Input / output format
Input:
```
n
r_1 r_2 ... r_n
```
Output: the minimum total, then one line per child (index, rating, candies).

**Input representation.** The ratings are kept as a single array. Only the *slope* between neighbours (up, flat, down) matters, so the line is seen as alternating runs of ascending and descending slopes. Two derived quantities are used:

* `L[i]` = length of the strictly increasing run of ratings that ends at i,
* `R[i]` = length of the strictly decreasing run of ratings that starts at i.

### Example 1 (`sample_input.txt`)
```
3
1 0 2
```
Output:
```
Minimum total candies = 5
child    rating       candies
1        1            2
2        0            1
3        2            2
```

### Example 2 (`sample_input2.txt`)
```
7
1 2 87 87 87 2 1
```
Output:
```
Minimum total candies = 13
child    rating       candies
1        1            1
2        2            2
3        87           3
4        87           1
5        87           3
6        2            2
7        1            1
```
The middle 87 has equal-rated neighbours, so it only needs 1 candy.

## 3. Solution description

### Two-pass (bi-directional) greedy
```
c[i] = 1 for all i
left -> right :   if r[i] > r[i-1]:  c[i] = c[i-1] + 1          (now c[i] = L[i])
right -> left :   if r[i] > r[i+1]:  c[i] = max(c[i], c[i+1] + 1) (now c[i] = max(L[i], R[i]))
answer = sum of c[i]
```
The left pass satisfies every "higher than the left neighbour" constraint, the right pass fixes the "higher than the right neighbour" constraints without breaking the left ones, because it only ever raises values.

### Why it is optimal
1. **Lower bound.** If the ratings increase strictly along L[i] children ending at i, each child needs at least one more candy than the previous one, so c[i] ≥ L[i]. Symmetrically c[i] ≥ R[i]. So any valid assignment has c[i] ≥ max(L[i], R[i]).
2. **Achievability.** The assignment c[i] = max(L[i], R[i]) is valid. Take r[i] > r[i+1]. Then L[i+1] = 1 (the run ending at i+1 restarts), so c[i+1] = max(1, R[i+1]) = R[i+1], and R[i] = R[i+1] + 1, hence c[i] ≥ R[i] = c[i+1] + 1. The case r[i] > r[i-1] is symmetric.
3. Because every c[i] equals its lower bound, the total is the minimum.

### One-pass slope version (O(1) extra space)
Keep only `up` (current ascending run), `down` (current descending run) and `peak` (the height reached at the last peak):
```
total = 1
for i = 1..n-1:
    if r[i] > r[i-1]:   up++; down = 0; peak = up;  total += 1 + up
    elif r[i] == r[i-1]: up = down = peak = 0;       total += 1
    else:               up = 0; down++;              total += down + (peak >= down ? 0 : 1)
```
While descending, each child gets `down` candies; when the descent becomes longer than the previous ascent, the peak child needs one extra candy, which is the `+1` term. This version only returns the total, not the individual values.

## 4. Validation (`./candy --test`)
1. **Brute force** (n ≤ 8, ratings with many ties): search *all* assignments with values 1..n, keep those that satisfy every constraint, and take the minimum sum. 1500/1500 pass for both the two-pass and the one-pass versions, and the assignment printed by two-pass is checked for validity.
2. **Independent baseline** (n ≤ 2000, random / few-distinct / sawtooth / alternating-sign patterns): process children in increasing rating order with `c[i] = 1 + max(c of a lower-rated neighbour)` (longest path in the constraint DAG). Two-pass, one-pass and this baseline all agree. 1000/1000 pass.
3. **Special cases:** strictly increasing and strictly decreasing (total n(n+1)/2), all equal (total n), alternating 0/1 (total 3n/2), n = 1, n = 0.

## 5. Complexity

| Algorithm | Time | Extra space |
|---|---|---|
| Two-pass (also returns each child's candies) | **O(n)** | O(n) for c[] |
| One-pass slope (total only) | **O(n)** | **O(1)** |
| Rating-order DAG baseline (validation only) | O(n log n) | O(n) |

* Every child is touched a constant number of times, so both algorithms are linear.
* Ω(n) is a lower bound because every rating must be read, so both are optimal in time.
* The total can reach about n²/2 (strictly monotone ratings), so a 64-bit total is used; the individual counts fit in an `int`.

Measured (`--bench`, random ratings): n = 400 000 → 0.003 s, 800 000 → 0.006 s, 1.6 M → 0.012 s, 3.2 M → 0.045 s for the two-pass version, i.e. linear growth; at 6.4 M the time rises to 0.23 s, most likely because of memory allocation and cache effects of the extra array. The one-pass version stays at 0.001 s to 0.02 s (about 0.003 µs per child) for the same inputs.

## 6. Assumptions
* Ratings are arbitrary integers (equal ratings are allowed, negative values are fine).
* "Immediate neighbours" means the child on the left and the child on the right; the line is not circular.
