# Minimum Refuelling Stops (and Minimum Initial Fuel by Reverse Greedy)

Files: `fuel.c` (solver + validation), `sample_input.txt`, `sample_input2.txt`, this README.

Build and run:
```
gcc -O2 -o fuel fuel.c
./fuel < sample_input.txt
./fuel --test     # validation
./fuel --bench    # empirical timing
```

## 1. Problem
A vehicle starts at 0 with F units of fuel (1 unit = 1 distance unit) and must reach distance D.
Station i is at distance d_i and gives f_i units of fuel when we stop there (any amount of stops, a stop at a station is used at most once).

* **Question asked:** the minimum number of refuelling stops needed to reach D (or report that it is impossible).
* **Reverse-greedy part (from the title):** the smallest initial fuel F* for which D is reachable at all.

## 2. Input / output format
Input:
```
D F n
d_1 f_1
...
d_n f_n
```
Output: the minimum number of stops, the stations used (distance and refuel amount), and the minimum initial fuel for which the target is reachable.

**Input representation.** The stations are stored as an array of pairs sorted by distance. The journey state is one number, `reach = F + (fuel taken so far)`, the farthest point the vehicle can reach. Stations already passed (d_i ≤ reach) but not yet used live in a **max-heap keyed by f_i**.

### Example 1 (`sample_input.txt`)
```
100 10 4
10 60
20 30
30 30
60 40
```
Output:
```
Minimum stops: 2
  stop at distance 10, refuel 60
  stop at distance 60, refuel 40
Minimum initial fuel for the target to be reachable at all: 10
```
reach = 10 → stop at 10 (+60) → reach 70 → the stations at 20, 30, 60 are now passed; the largest is 40 at distance 60 → reach 110 ≥ 100.

### Example 2 (`sample_input2.txt`) – impossible
```
100 5 3
10 60
30 40
70 20
```
Output:
```
Minimum stops: impossible (target cannot be reached with F = 5)
Minimum initial fuel for the target to be reachable at all: 10
```

## 3. Solution description

### A. Minimum number of stops – "refuel retroactively" greedy
```
reach = F, stops = 0, heap = empty
while reach < D:
    push every unseen station with d <= reach into the max-heap
    if heap is empty: impossible
    pop the station with the largest f;  reach += f;  stops += 1
```
**Idea.** We never decide to stop at a station while driving. We only stop when forced (we would run out of fuel before the next point) and then act as if we had refuelled at the best station already passed.

**Why it is optimal (exchange argument).** Let the greedy have used k stops and reach R_k. Claim: R_k is the maximum reach possible with k stops. Induction: with k stops any strategy picks k stations that were reachable, and the sum of its k refuels is at most the sum of the k largest refuels among reachable stations. Greedy takes the largest available at every step, so its reach after each stop is maximal, hence it reaches D with the fewest stops. If the heap is empty while reach < D, no unused station lies within reach, so reach cannot grow: the target is unreachable.

### B. Minimum initial fuel – reverse greedy
Taking a station never hurts (it only adds fuel), so to find the smallest feasible F we may assume all stations before D are used. Scan **backwards** from the target and keep R = the minimum fuel we must hold when *arriving* at the next point:

```
R = 0, next = D
for stations j from the last (d_j < D) to the first:
    R = max(0, (next − d_j) + R − f_j)     // arrive at j with R, refuel f_j, then cover the gap
    next = d_j
F* = next + R                              // from the origin, no refuel
```
This is exact because the fuel needed on arrival is the larger of 0 and "gap + needed at the next point − refuel here", and a smaller value at one station can only help the earlier ones. The program checks it: the target is reachable with F* and not with F* − 1.

## 4. Validation (`./fuel --test`)
1. **Brute force** (n ≤ 12): try every subset of stations, simulate the trip, take the smallest feasible subset size. Dense and sparse random instances including F = 0, stations at distance 0, stations beyond D. 1500/1500 pass. The plan printed by the greedy is also re-simulated and must be valid.
2. **Independent DP baseline** (n ≤ 300): `dp[k]` = maximum reach with exactly k stops, answer = smallest k with `dp[k] ≥ D`. 600/600 pass.
3. **Reverse greedy**: for random instances F* is feasible and F* − 1 is infeasible according to the stops algorithm. 1500/1500 pass.

## 5. Complexity

| Part | Time | Space |
|---|---|---|
| Sort stations by distance | O(n log n) | O(n) |
| Greedy: each station pushed once, popped at most once, O(log n) each | O(n log n) | O(n) |
| Reverse greedy for F* | O(n) (after sorting) | O(1) |
| **Total** | **O(n log n)** | **O(n)** |

* Number of stops ≤ n, so the loop runs at most n times.
* The DP alternative is O(n²) time, O(n) space.
* If stations are already sorted by distance, the whole thing is the heap work only, O(n log n).

Measured (`--bench`, random stations, 600 000 stops at the largest size): n = 100 000 → 0.016 s, 200 000 → 0.035 s, 400 000 → 0.18 s, 800 000 → 0.22 s, 1 600 000 → 0.43 s, which is consistent with n log n (the 400 000 point is timing noise).

## 6. Assumptions
* Distances and fuel are non-negative integers; fuel is consumed 1 per distance unit; the tank has no capacity limit.
* The vehicle arriving with exactly 0 fuel at a station may still refuel there; reaching D with exactly 0 fuel counts as success.
* Stations at distance ≥ D are ignored.
