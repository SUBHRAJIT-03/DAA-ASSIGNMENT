# Q01 - Online Graph-Bipartiteness Testing

## Question Title
[Online Graph-Bipartiteness Testing] A graph is bipartite if and only if it contains no odd-length cycles. Given an undirected graph with $V$ vertices, edges arrive online one by one. Maintain whether the graph remains bipartite. If adding an edge $(u, v)$ does not introduce an odd cycle, add it; otherwise, report that it violates bipartiteness and skip it. By choosing the proper input representation, write a program in C to validate your algorithm and derive the complexity analysis of your algorithm.

## How the Code Works
The program starts with a menu prompting the user for either manual edge-stream entry (`handleManualInput`) or randomized graph generation (`generateRandomData`). Each arriving edge $(u, v)$ is processed sequentially by `processOnlineEdges`, which invokes `addEdgeOnline` on a Parity-Augmented Disjoint Set Union (`BipartiteDSU`) structure. For every edge, `findSet` compresses the path while accumulating XOR parities to the component root, accepting the edge if it connects two different components or two vertices of opposite parity, and skipping it if it forms an odd cycle.

## Maths / Logic Behind This
An undirected graph is bipartite if and only if its vertices can be 2-colored (assigned colors in $\{0, 1\}$) such that every edge connects vertices of opposite colors. Rather than re-running Breadth-First Search (BFS) on the entire graph after each edge arrival, we augment a Disjoint Set Union (DSU) data structure with a relative parity array `parity[x]`, which stores the XOR difference ($\oplus$) between the color of vertex $x$ and the color of its parent `parent[x]`.

1. **Path Compression with XOR Potentials:** For any vertex $x$, the XOR sum of `parity` values along the path from $x$ to its component representative $r = \text{find}(x)$ gives the relative color difference $d(x) = \text{color}(x) \oplus \text{color}(r) \in \{0, 1\}$. During path compression, we recursively find the root of `parent[x]` and update:
   $$\text{parity}[x] \leftarrow \text{parity}[x] \oplus \text{parity}[\text{old\_parent}[x]]$$
2. **Cycle Parity Check:** When an online edge $(u, v)$ arrives, we compute $(r_u, d(u)) = \text{findSet}(u)$ and $(r_v, d(v)) = \text{findSet}(v)$:
   - **Same Component ($r_u = r_v$):** If $d(u) = d(v)$, then $u$ and $v$ have the same color in the unique 2-coloring of their connected component, meaning the existing path between $u$ and $v$ has even length and adding $(u, v)$ creates an **odd-length cycle**. We report a bipartiteness violation and skip the edge. If $d(u) \neq d(v)$, the edge is consistent with the existing 2-coloring.
   - **Different Components ($r_u \neq r_v$):** Merging two bipartite components via a single edge never creates a cycle. To force $\text{color}(u) \oplus \text{color}(v) = 1$, the relative parity between the two roots $r_u$ and $r_v$ must satisfy:
     $$\text{parity}[\text{child\_root}] = d(u) \oplus d(v) \oplus 1$$

## Complexity Analysis
- **Time complexity:** $\mathcal{O}(V + E \cdot \alpha(V))$, where $V$ is the number of vertices, $E$ is the number of online edges, and $\alpha(V)$ is the inverse Ackermann function (practically constant, $\alpha(V) \le 4$, per edge operation due to union by rank and path compression).
- **Extra space complexity:** $\mathcal{O}(V)$ auxiliary space to store the `parent`, `rank`, and `parity` arrays of the DSU (plus $\mathcal{O}(E)$ space to buffer the input stream of edges).
