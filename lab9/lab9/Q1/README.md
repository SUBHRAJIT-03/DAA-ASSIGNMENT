# Fractional Knapsack with Deterioration Rate

Files: `decay_knapsack.c` (solver + validation), `sample_input.txt`, `sample_input2.txt`, this README.

Build and run:
```
gcc -O2 -o decay_knapsack decay_knapsack.c -lm
./decay_knapsack < sample_input.txt
./decay_knapsack --test     # validation
./decay_knapsack --bench    # empirical timing
```

## 1. Problem model

Item i has base value v_i, weight w_i, decay rate λ_i > 0, and density d_i = v_i / w_i.
Items are consumed one after another at speed 1 weight-unit per time-unit, so **time t = weight consumed so far**.
One unit of weight consumed at time t is worth `d_i − λ_i·t`.

If we take x_i (0 ≤ x_i ≤ w_i) of item i during [s_i, e_i], e_i = s_i + x_i:

    gain_i = ∫ from s_i to e_i of (d_i − λ_i t) dt = d_i·x_i − λ_i (s_i·x_i + x_i²/2)

Goal: choose an order and the amounts x_i, with Σ x_i ≤ W, to maximise Σ gain_i.

**Input representation.** Each item is stored as a *line* (d_i, λ_i) in the (time, density) plane, `density_i(t) = d_i − λ_i t`.
The weight axis doubles as the time axis, so a schedule is a partition of [0, T] into consecutive blocks.

## 2. Input / output format

Input:
```
n W
v_1 w_1 lambda_1
...
v_n w_n lambda_n
```
Output: the schedule in consumption order (item id, density, λ, weight taken, fraction, start, end, gain), total weight used and the maximum value.

### Example 1 (`sample_input.txt`)
```
4 10
60 5 0.8
50 4 0.3
90 6 1.2
20 5 0.1
```
Output:
```
ord   item   density   lambda   weight    fraction  start     end       gain
1     3      15.0000   1.2000   6.0000    1.0000    0.0000    6.0000    68.4000
2     1      12.0000   0.8000   0.0000    0.0000    6.0000    6.0000    0.0000
3     2      12.5000   0.3000   4.0000    1.0000    6.0000    10.0000   40.4000
4     4      4.0000    0.1000   0.0000    0.0000    10.0000   10.0000   0.0000
Total weight used = 10.0000 / 10.0000
Maximum total value = 108.800000
```
Item 1 has a higher density than the cheap-decay items but decays fast. It loses to item 3 (denser and decays even faster, so it must go first) and to item 2 (slow decay keeps its worth late).

### Example 2 (`sample_input2.txt`) – fractional choices
```
3 8
100 5 2.0
72 6 0.5
40 4 0.2
```
Output:
```
ord   item   density   lambda   weight    fraction  start     end       gain
1     1      20.0000   2.0000   5.0000    1.0000    0.0000    5.0000    75.0000
2     2      12.0000   0.5000   1.6667    0.2778    5.0000    6.6667    15.1389
3     3      10.0000   0.2000   1.3333    0.3333    6.6667    8.0000    11.3778
Total weight used = 8.0000 / 8.0000
Maximum total value = 101.516667
```

## 3. Solution description

### Step 1 – Optimal order: sort by λ descending
Take two unit slots at times t1 < t2 holding items i and j. Value before swapping minus value after is
`(λ_i − λ_j)(t2 − t1)`.
So if λ_i > λ_j, item i should be in the earlier slot. Repeated swaps give: **consume items in non-increasing λ** (an item that decays faster is used earlier).
Also, shifting any block earlier never hurts because every density decreases in t, so the schedule has no idle gaps and starts at t = 0.

(Greedy by current highest density is *not* optimal; e.g. d=10, λ=0 and d=9, λ=5, w=1 each, W=2 gives 14 for greedy but 16.5 when the fast-decaying item goes first.)

### Step 2 – Choose the amounts (exact)
With the order fixed, let e_i be the end time of the i-th item in the order (e_0 = 0), F_i(t) = d_i t − λ_i t²/2. Then

    total = Σ [F_i(e_i) − F_i(e_{i−1})] = Σ_{i<n} G_i(e_i) + F_n(e_n),
    G_i = F_i − F_{i+1}  (G_i'' = −(λ_i − λ_{i+1}) ≤ 0)

so the objective is a **separable concave function of the end times**, with constraints
`e_{i−1} ≤ e_i ≤ e_{i−1} + w_i`, `e_n ≤ W`. (The fractions are then x_i = e_i − e_{i−1}, and the order sorted by λ makes G_i concave.)

Define B_i(e) = best value of the first i items with e_i = e. Then

    B_i(e) = G_i(e) + max over y in [e − w_i, e] of B_{i−1}(y)

B_{i−1} is concave, so the window maximum equals B_{i−1} left of its peak y*, is flat on [y*, y* + w_i], and is B_{i−1}(e − w_i) to the right.
In terms of the derivative (stored as a list of linear pieces `a + b·e`): find the zero y*, cut there, insert a zero piece of length w_i, shift the right part by w_i, and add the linear function G_i'.
Each step adds at most 2 pieces. After item n, pick the best e_n in [0, min(W, Σw)] (first zero of the derivative, or the limit) and **back-track** with `e_{i−1} = clamp(y*_i, e_i − w_i, e_i)`.

Because everything is concave the result is the global optimum, and the solution can stop early when densities become negative.

## 4. Validation (`./decay_knapsack --test`)
1. **Brute force** (n ≤ 3): all n! orders × a grid of amounts; the algorithm must be ≥ every grid point. 300/300 random cases pass (including ties in λ).
2. **KKT optimality certificate** (n ≤ 60): for a concave program on this polytope the solution is optimal iff no single increase, decrease or pairwise transfer of weight improves the objective; this is checked exactly from the gradient `g_i = d_i − λ_i e_i − Σ_{j>i} λ_j x_j`. 2000/2000 random cases pass.

## 5. Complexity

| Part | Time | Space |
|---|---|---|
| Read input / compute densities | O(n) | O(n) |
| Sort by λ | O(n log n) | O(1) extra |
| DP: per item one scan, one rebuild of ≤ i+1 pieces | O(n) per item, **O(n²)** total | O(n) |
| Back-track | O(n) | O(n) |
| **Total** | **O(n²)** | **O(n)** |

Measured (`--bench`): n = 2000 → 0.006 s, 4000 → 0.03 s, 8000 → 0.12 s, 16000 → 0.52 s, 32000 → 2.6 s, i.e. about ×4 per doubling, which is quadratic.

The O(n²) comes only from the array representation of the derivative. Keeping the pieces in a balanced tree with lazy "add linear function" and "shift" tags (search by derivative sign, split, insert) brings the DP to **O(n log n)**, so the whole algorithm becomes O(n log n). The simpler array version was implemented because it is easy to verify.

## 6. Assumptions
* Consumption speed is 1 weight-unit per time-unit, so the time of consumption equals the cumulative weight before it; the value of a fraction is the integral of the decaying density over the time it occupies.
* Densities may become negative later in time; the optimum then leaves the knapsack partly empty (total weight used < W).
