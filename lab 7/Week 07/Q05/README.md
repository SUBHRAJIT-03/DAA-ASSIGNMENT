## Q05 - Hitting a Moving Target

## How the Code Works
The program asks whether the user wants to enter `n` manually or generate a random value. It then models the possible target positions as a belief set using bitmasks. Each bit represents a possible hiding place among `n` positions.

The code performs a depth-limited DFS search. At each step, it tries shooting at each possible position, removes that possibility from the belief set, and then simulates the target moving to adjacent positions. If a strategy reaches a state where the target is guaranteed to be hit in the remaining steps, the search succeeds.

## Maths / Logic Behind This
The target can move only to neighboring positions. Therefore, after removing a shot position, the remaining possible target positions are updated by taking the neighbor set of each surviving state.

The program uses a memoized recursive search to avoid recomputing identical belief states. The general idea is to check whether there exists a sequence of shots that guarantees a hit against all adversarial moves.

## Complexity Analysis
- Time Complexity: exponential in the number of belief states
- Space Complexity: O(2^n)

This is a search problem over possible target distributions, and the state space grows rapidly as `n` increases.

## Output
The program reports whether a guaranteed strategy was found within the allowed search depth. If no strategy is found, it prints that no guaranteed strategy exists up to the tested depth.
