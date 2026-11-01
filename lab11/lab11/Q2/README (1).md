# Q02 - Multi-Source BFS for Radius-Constrained Voronoi Cell Partitioning

## Question Title
**[Multi-Source BFS for Radius-Constrained Voronoi Cell Partitioning]** You are given:
- An unweighted, undirected graph $G = (V, E)$.
- A set of $K$ designated hub vertices $H = \{h_1, h_2, \dots, h_K\} \subseteq V$, indexed from $1$ to $K$.
- An integer radius threshold $R \ge 0$.

Let $\text{dist}(u, v)$ denote the shortest path distance (minimum number of edges) between vertices $u$ and $v$ in $G$.
For every vertex $v \in V$, assign an ownership label $\text{owner}[v] \in \{1, 2, \dots, K\} \cup \{-1\}$ according to the following rules:
1. *Distance Metric:* Find the distance from $v$ to each service hub: $d_i = \text{dist}(v, h_i)$ for all $i \in \{1, \dots, K\}$. Let $d_{\min}(v) = \min_{1 \le i \le K} d_i$.
2. *Reachability Threshold ($R$):* If $d_{\min}(v) > R$, vertex $v$ lies outside the service coverage area. Set $\text{owner}[v] = -1$.
3. *Closest Hub Assignment:* If $d_{\min}(v) \le R$, assign $v$ to the hub $h_i$ that minimizes $d_i$.
4. *Deterministic Tie-Breaking:* If multiple hubs achieve the exact same minimum distance $d_{\min}(v)$, choose the hub with the smallest numerical index $i$.

*Objective:* Output the array $\text{owner}[1 \dots |V|]$ containing the assigned hub index (or $-1$ if unassigned) and the corresponding shortest distance to that hub for every vertex. By choosing the proper input representation, write a program in C to validate your algorithm and derive the complexity analysis of your algorithm.

## How the Code Works
The program prompts the user to select manual input or randomized graph and hub generation, constructing an undirected adjacency list for $G = (V, E)$. It initializes all vertices as unassigned (`owner[v] = -1`, `dist[v] = INF`) and seeds a single FIFO queue with the $K$ hubs in strictly increasing order of their indices $1, 2, \dots, K$. A single Multi-Source BFS pass then propagates hub ownership layer-by-layer up to distance $R$, automatically breaking distance ties in favor of the smallest hub index and printing the final `owner[1...|V|]` and distance table.

## Maths / Logic Behind This
Running $K$ independent BFS traversals from each hub $h_i$ would require $\mathcal{O}(K(|V| + |E|))$ time. Instead, we can conceptually introduce a virtual super-source $S^*$ connected to each hub $h_i \in H$ via a zero-weight directed edge, reducing the problem to a single **Multi-Source BFS** that simultaneously expands wave-fronts from all $K$ hubs.

Two properties guarantee exact compliance with the Voronoi partitioning rules:
1. **Deterministic Tie-Breaking by Queue Ordering**: Because all hubs $h_1, h_2, \dots, h_K$ start at distance $0$ and are enqueued in increasing order of their hub index $i$, induction on the BFS level $d$ shows that the queue is partitioned by non-decreasing distance $d$, and within each distance layer $d$, vertices are ordered by non-decreasing `owner[u]`. Consequently, the first time an unvisited vertex $v$ is reached at distance $d_{\min}(v) = d + 1$, it is discovered by a neighbor $u$ at distance $d$ carrying the minimum possible hub index $i$.
2. **Radius Pruning ($d_{\min}(v) \le R$)**: Whenever a dequeued vertex $u$ has $\text{dist}[u] = R$, its neighbors are not expanded because any path through $u$ to an unvisited vertex would have distance $R + 1 > R$. Thus, any vertex with $d_{\min}(v) > R$ remains unvisited with $\text{owner}[v] = -1$.

## Complexity Analysis
- Time complexity: $\mathcal{O}(|V| + |E|)$ — Every vertex is enqueued at most once and every undirected edge is inspected at most twice during the single Multi-Source BFS traversal.
- Extra space complexity: $\mathcal{O}(|V| + |E|)$ total space ($\mathcal{O}(|V|)$ auxiliary space for the `dist`, `owner`, and FIFO `queue` arrays of size $|V|$, plus $\mathcal{O}(|V| + |E|)$ for the adjacency list).
