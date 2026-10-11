# Minimum Number of Meeting Rooms

Files: `meeting_rooms.c` (solver + validation), `sample_input.txt`, `sample_input2.txt`, this README.

Build and run:
```
gcc -O2 -o meeting_rooms meeting_rooms.c
./meeting_rooms < sample_input.txt
./meeting_rooms --test     # validation
./meeting_rooms --bench    # empirical timing
```

## 1. Problem
Given meeting intervals [s_i, e_i], find the minimum number of rooms so that no two meetings in the same room overlap.
Intervals are half-open, [s, e), so a meeting that ends at time t and another that starts at t can use the same room. Each meeting needs s < e.

## 2. Input / output format
Input:
```
n
s_1 e_1
...
s_n e_n
```
Output: the minimum number of rooms, then every meeting (in input order) with the room it is assigned to.

**Input representation.** For *counting* rooms, which start belongs to which end does not matter; only the number of meetings in progress at each moment does. So the intervals are split into two plain arrays, the **starts S[]** and the **ends E[]**, each sorted on its own and then swept like the two halves of a merge. (For producing an actual schedule the meetings are kept as records and sorted by start time.)

### Example 1 (`sample_input.txt`)
```
3
0 30
5 10
15 20
```
Output:
```
Minimum number of rooms = 2
meeting  start        end          room
1        0            30           1
2        5            10           2
3        15           20           2
```

### Example 2 (`sample_input2.txt`)
```
8
9 10
4 9
4 17
1 5
5 8
7 12
10 15
14 16
```
Output:
```
Minimum number of rooms = 4
meeting  start        end          room
1        9            10           1
2        4            9            2
3        4            17           3
4        1            5            1
5        5            8            1
6        7            12           4
7        10           15           1
8        14           16           2
```
Meetings 2, 3, 5 and 6 are all in progress around time 7 to 8, so 4 rooms are unavoidable.

## 3. Solution description

### A. Counting rooms: endpoint sweep
```
sort S ascending;  sort E ascending
i = j = 0;  cur = 0;  best = 0
while i < n:
    if S[i] < E[j]:  cur++, i++, best = max(best, cur)   // a meeting starts before the next one ends
    else:            cur--, j++                          // a meeting ends (ties: the end is handled first)
answer = best
```
`cur` is the number of meetings in progress. Because an end equal to a start is processed first, touching meetings do not count as overlapping.

### B. Producing a schedule: sort by start + min-heap
Sort meetings by start time. Keep a min-heap of busy rooms keyed by end time, and a min-heap of free room numbers.
For each meeting: release every room whose end time ≤ this start (move it to the free heap); take the smallest free room, or open a new room if none is free; push (end, room).

### Why it is optimal
* **Lower bound.** At any time t, the meetings with s ≤ t < e must all be in different rooms, so the answer is at least the maximum number of simultaneous meetings. The maximum is always attained at some meeting's start time, and the sweep computes exactly this maximum.
* **Achievability.** In the schedule of part B a new room is opened only when every existing room is busy at the new meeting's start time s. Then those rooms' meetings plus the new one all contain the time s, so at least (rooms + 1) meetings overlap at s. Hence the number of rooms opened never exceeds the maximum overlap, and by the lower bound it equals it. (This is colouring an interval graph, where the chromatic number equals the clique number.)

## 4. Validation (`./meeting_rooms --test`)
1. **Exhaustive search** (n ≤ 7, times 0..10 with many touching and tied endpoints, 3000 cases): a backtracking search finds the smallest k for which the meetings can be assigned to k rooms without a conflict, with no use of the "maximum overlap" idea. The sweep, the heap schedule and the O(n²) overlap count all equal it, and the produced schedule is checked for conflicts (no two overlapping meetings in the same room). 3000/3000 pass.
2. **Larger inputs** (n ≤ 1500, dense, sparse, short and long meetings, all-identical meetings): sweep = heap schedule = O(n²) baseline = difference array (`+1` at s, `−1` at e, prefix maximum), and every schedule is conflict-free. 600/600 pass.
3. **Special cases:** the two classic examples, touching meetings `[1,2],[2,3]` → 1, nested meetings → 4, 100 identical meetings → 100, a mix of touching and long meetings → 2, n = 0 → 0.

## 5. Complexity

| Part | Time | Space |
|---|---|---|
| Sort the starts and the ends | O(n log n) | O(n) |
| Sweep (two pointers) | **O(n)** | O(1) extra |
| **Count only (total)** | **O(n log n)** | **O(n)** |
| Schedule: sort by start + heaps (each meeting pushed/popped once per heap) | **O(n log n)** | O(n) |
| O(n²) overlap count (baseline) | O(n²) | O(1) |
| Difference array, integer times in [0, T) | O(n + T) | O(T) |

* The sort dominates; the sweep itself is linear.
* If the times are small integers (a bounded range T), the difference array or a counting sort gives O(n + T).
* The answer is at most n, and the heap never holds more than the number of rooms in use, so the heap sizes equal the answer.

Measured (`--bench`): random times up to 10^9, n = 100 000 → 0.029 s, 200 000 → 0.050 s, 400 000 → 0.107 s, 800 000 → 0.22 s, 1.6 M → 0.47 s for the sweep (n log n growth); the heap schedule is about 20–25 % slower (0.033 s to 0.57 s) but also returns the assignment. With small integer times (T = 10^6) the difference array handles 3.2 M meetings in 0.018 s, while the sort-based sweep needs about 1.0 s, which illustrates the O(n + T) alternative.

## 6. Assumptions
* Times are integers (64-bit) and every meeting has start < end; touching meetings (end = start of another) may share a room.
* The room numbers in the schedule are 1-based and chosen as the smallest free room; any conflict-free assignment with the minimum number of rooms is acceptable.
