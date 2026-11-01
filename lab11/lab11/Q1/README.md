# Q01 - Shortest Path on Parity-Constrained Graphs via 0-1 BFS

## Question Title
**[Shortest Path on Parity-Constrained Graphs via 0-1 BFS]** You are given a directed graph $G = (V, E)$, a designated source vertex $S \in V$, and an extra penalty cost $C \ge 0$. Every directed edge $(u, v) \in E$ has a weight $w(u, v) \in \{0, 1\}$.

For any path $P$ from $S$ to a vertex $v$:
1. Let $W(P)$ denote the sum of all edge weights along $P$.
2. Let $k_1(P)$ denote the total count of weight-1 edges traversed in $P$.
3. The effective cost of path $P$ is defined as:
   $$\text{Cost}(P) = W(P) + \begin{cases} 0, & \text{if } k_1(P) \text{ is even} \\ C, & \text{if } k_1(P) \text{ is odd} \end{cases}$$

*Objective:* For every vertex $v \in V$, compute the minimum possible $\text{Cost}(P)$ among all valid paths from $S$ to $v$. If a node $v$ cannot be reached from $S$, report that it is unreachable. By choosing the proper input representation, write a program in C to validate your algorithm and derive the complexity analysis of your algorithm.

## How the Code Works
The program starts with a menu-driven prompt allowing either manual graph entry or randomized directed 0-1 graph generation, storing the graph in a dynamic adjacency list. It then runs a state-space 0-1 BFS using a double-ended queue (`Deque`) over state pairs `(vertex, parity)` to compute the shortest path weights for both even and odd counts of weight-1 edges. Finally, for every vertex $v \in V$, it compares the minimum even-parity path weight against the minimum odd-parity path weight plus $C$, printing a formatted table of results or reporting `UNREACHABLE`.

## Maths / Logic Behind This
Because every edge weight in $G$ belongs to $\{0, 1\}$, the path weight $W(P)$ equals the count of weight-1 edges $k_1(P)$. However, the non-monotonic penalty $+C$ applied only when $k_1(P) \equiv 1 \pmod 2$ means a path with a slightly larger even $W(P)$ can beat a path with a smaller odd $W(P)$ when $C$ is large.

To restore the optimal substructure property required for shortest-path search, we split each vertex $v \in V$ into two parity states:
- $(v, 0)$: reaching vertex $v$ via a path $P$ with $k_1(P) \equiv 0 \pmod 2$.
- $(v, 1)$: reaching vertex $v$ via a path $P$ with $k_1(P) \equiv 1 \pmod 2$.

For a fixed parity $p \in \{0, 1\}$, the effective cost $\text{Cost}(P) = W(P) + p \cdot C$ is strictly increasing with $W(P)$. Each directed edge $(u, v)$ of weight $w \in \{0, 1\}$ induces state transitions:
$$(u, p) \xrightarrow{w} (v, p \oplus w)$$
Since all transition weights in this expanded state graph $G' = (V \times \{0, 1\}, E')$ are in $\{0, 1\}$, **0-1 BFS** finds the minimum path weight $d[v][0]$ and $d[v][1]$ for all vertices in linear time by pushing weight-$0$ transitions to the front of the deque and weight-$1$ transitions to the back. The minimum effective cost for vertex $v$ is then:
$$\min\big(d[v][0], \; d[v][1] + C\big)$$

## Complexity Analysis
- Time complexity: $\mathcal{O}(|V| + |E|)$ — There are $2|V|$ parity states and $2|E|$ state transitions, each expanded at most once in $\mathcal{O}(1)$ time per deque operation.
- Extra space complexity: $\mathcal{O}(|V| + |E|)$ total space ($\mathcal{O}(|V|)$ auxiliary space for the $2|V|$ distance/visited tables and deque, plus $\mathcal{O}(|V| + |E|)$ for the adjacency list representation).
