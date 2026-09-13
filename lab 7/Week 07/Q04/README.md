## Q04 - Security Switches

## How the Code Works
The program starts by asking whether the user wants a manual value for `n` or a random one. It then treats each switch state as a binary mask, where each bit represents one switch.

The program uses Breadth-First Search (BFS) on the state space of all possible switch configurations. Each state is transformed by toggling a valid switch according to the problem rule. BFS guarantees the shortest path, so the first time the program reaches the all-off state, it has found the minimum number of moves.

## Maths / Logic Behind This
Each switch configuration is represented by a bitmask. The number of possible states is:

2^n

A toggle is legal only when:
- the current switch is the rightmost one, or
- the switch immediately to its right is ON and all switches further right are OFF.

The BFS explores the graph of states, where each edge corresponds to one legal toggle. The shortest path from the initial all-ON state to the all-OFF state gives the minimum sequence of operations.

## Complexity Analysis
- Time Complexity: O(2^n × n)
- Space Complexity: O(2^n)

This is because the BFS explores at most all possible switch states and checks legal toggles from each state.

## Output
The program prints:
- the minimum number of toggles required to turn all switches off
- the sequence of switch indices (left-indexed from 1 to `n`) that achieves the solution
