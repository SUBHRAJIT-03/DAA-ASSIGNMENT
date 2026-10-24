# Q04 - Reverse Time-Step DSU Application

## Question Title
[Reverse Time-Step DSU Application] Given a vertex-weighted undirected graph, vertices are sequentially deleted one by one in an order $P = [p_1, p_2, \dots, p_V]$. After each vertex deletion, compute the maximum sum of vertex values among all remaining connected components. Note that deleting a vertex in a graph means removing that specific vertex and all edges connected (incident) to it from the graph structure. By choosing the proper input representation, write a program in C to validate your algorithm and derive the complexity analysis of your algorithm.

## How the Code Works
The program prompts the user to select either manual input (`handleManualInput`) or randomized graph and permutation generation (`generateRandomData`). The undirected graph is stored in an adjacency list (`Graph`), and the deletion sequence $P = [p_1, p_2, \dots, p_V]$ is processed in **reverse chronological order** inside `processReverseDeletions`. Starting from an empty graph, vertices are activated one by one from $p_V$ down to $p_1$, uniting each newly activated vertex with its already-active neighbors via `unionSets` while maintaining the running maximum component sum and recording the answer for each step before outputting the results in forward order.

## Maths / Logic Behind This
Standard Disjoint Set Union (DSU) data structures support fast incremental edge/vertex additions ($\mathcal{O}(\alpha(V))$) but cannot efficiently split components upon vertex or edge deletions. Because the entire deletion permutation $P = [p_1, p_2, \dots, p_V]$ is known upfront, we transform **sequential deletions** into **sequential additions** via **Offline Reverse Time-Step Processing**:

1. **State Equivalence:** The subgraph remaining immediately *after* deleting $\{p_1, p_2, \dots, p_t\}$ is identical to the subgraph induced by the vertex set $\{p_{t+1}, p_{t+2}, \dots, p_V\}$, which is the exact state of the reverse process right *before* activating $p_t$.
2. **Reverse Addition Loop ($t = V$ down to $1$):**
   - Store the current global maximum component weight `ans[t] = current_max_sum` (for $t = V$, no vertices are active, so `ans[V] = 0`).
   - Mark vertex $u = p_t$ as active (`active[u] = true`) and update `current_max_sum = max(current_max_sum, weight[u])`.
   - Iterate over all neighbors $v \in \text{Adj}(u)$. If `active[v]` is true, merge the components of $u$ and $v$ in the DSU:
     $$W[r_{\text{merged}}] = W[\text{find}(u)] + W[\text{find}(v)]$$
     and update `current_max_sum = max(current_max_sum, W[r_merged])`.
3. **Monotonicity of Maximum Sum:** Because all vertex weights are non-negative and components only grow as vertices and incident edges are added in reverse time, `current_max_sum` is monotonically non-decreasing backward in time and can be maintained in $\mathcal{O}(1)$ per union.

## Complexity Analysis
- **Time complexity:** $\mathcal{O}((V + E) \cdot \alpha(V))$, since each of the $V$ vertices is activated once and each of the $E$ undirected edges is inspected at most twice (once from each endpoint), triggering at most one DSU union in $\mathcal{O}(\alpha(V))$ amortized time.
- **Extra space complexity:** $\mathcal{O}(V + E)$ auxiliary space to store the adjacency list of the graph, the DSU arrays (`parent`, `rank`, `comp_weight`), the `active` boolean array, and the `ans_after_deletion` output array.
