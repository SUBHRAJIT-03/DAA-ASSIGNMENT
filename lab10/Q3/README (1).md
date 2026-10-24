# Q03 - Maximum Weight Component with Size Constraints

## Question Title
[Maximum Weight Component with Size Constraints] Given a graph where each vertex $v$ has weight $w_v$, edges are added sequentially with weights. At any point, compute the maximum total vertex weight of any connected component whose size does not exceed a threshold $K$. By choosing the proper input representation, write a program in C to validate your algorithm and derive the complexity analysis of your algorithm.

## How the Code Works
At startup, the user chooses between manual entry (`handleManualInput`) or randomized graph generation (`generateRandomData`). The program initializes a `WeightedDSU` structure where each vertex $v \in \{1, \dots, V\}$ starts as a singleton component of size $1$ and vertex weight $w_v$. As each weighted edge $(u, v, w_e)$ arrives in `processSequentialEdges`, `unionComponents` merges the two endpoint sets (updating both the component vertex count and total vertex weight), and `queryMaxValidComponentWeight` evaluates all active component roots $r$ satisfying $\text{size}[r] \le K$ to report the maximum total vertex weight.

## Maths / Logic Behind This
We maintain the connected components dynamically using an augmented Disjoint Set Union (DSU) data structure:
1. **Component Invariants:** For every active component representative (root) $r$, the DSU stores:
   $$\text{size}[r] = |C_r|, \quad W[r] = \sum_{v \in C_r} w_v$$
   Initially, for each vertex $v \in \{1, \dots, V\}$, $\text{parent}[v] = v$, $\text{size}[v] = 1$, and $W[v] = w_v$.
2. **State Transition on Edge Addition:** When an edge $(u, v, w_e)$ arrives, we locate $r_u = \text{findSet}(u)$ and $r_v = \text{findSet}(v)$. If $r_u \neq r_v$, union by rank attaches the smaller-rank root to the larger-rank root $r_{\text{new}}$ and updates:
   $$\text{size}[r_{\text{new}}] \leftarrow \text{size}[r_u] + \text{size}[r_v], \quad W[r_{\text{new}}] \leftarrow W[r_u] + W[r_v]$$
3. **Non-Monotonicity Under Threshold $K$:** Unlike unconstrained maximum-weight tracking, merging two components of sizes $s_1 \le K$ and $s_2 \le K$ can produce a merged component of size $s_1 + s_2 > K$, which **disqualifies** the merged component from the feasible set $\{C_r \mid \text{size}[r] \le K\}$. Therefore, a simple running maximum variable is insufficient; after each edge arrival, we evaluate:
   $$W_{\max}(K) = \max_{\substack{1 \le r \le V \\ \text{parent}[r] = r, \; \text{size}[r] \le K}} W[r]$$
   (which can be computed by scanning roots in $\mathcal{O}(V)$ per query or maintained in $\mathcal{O}(\log V)$ per step using a balanced binary search tree / lazy max-heap).

## Complexity Analysis
- **Time complexity:** $\mathcal{O}(E \cdot (\alpha(V) + V))$ using direct root scanning after each of the $E$ edge additions (where each DSU union takes $\mathcal{O}(\alpha(V))$ time and scanning active roots takes $\mathcal{O}(V)$ time; if only $Q$ queries are issued, it takes $\mathcal{O}(E \cdot \alpha(V) + Q \cdot V)$, or $\mathcal{O}((V + E) \log V)$ with an auxiliary balanced BST/heap).
- **Extra space complexity:** $\mathcal{O}(V)$ auxiliary space for the DSU arrays (`parent`, `rank`, `comp_size`, `comp_vertex_weight`, and `comp_edge_weight`).
