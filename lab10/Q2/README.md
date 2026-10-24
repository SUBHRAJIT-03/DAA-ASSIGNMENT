# Q02 - Dynamic Connected Components in a Grid

## Question Title
[Dynamic Connected Components in a Grid] Given an $R \times C$ grid initially completely filled with water, land cells are added one by one at positions $(r_1, c_1), (r_2, c_2), \dots, (r_k, c_k)$. After each operation, output the current number of 4-directionally connected land components (islands). By choosing the proper input representation, write a program in C to validate your algorithm and derive the complexity analysis of your algorithm.

[Hint: This is the Dynamic Connected Components in a Grid (commonly known as Number of Islands II), which can be solved using a Disjoint Set Union (DSU / Union-Find) data structure with coordinate flattening.]

## How the Code Works
The program prompts the user via a menu to either enter the grid dimensions and $k$ land coordinates manually (`handleManualInput`) or generate a random test sequence (`generateRandomData`). In `simulateGridOperations`, an $R \times C$ Disjoint Set Union (`GridDSU`) structure is initialized with all cells marked as water. Each operation calls `addLandCell`, which flattens the 2D coordinate $(r, c)$ to a 1D index, marks the cell as land, increments the island counter, checks its 4-directional neighbors, and decrements the island counter whenever `unionSets` merges two distinct adjacent land components.

## Maths / Logic Behind This
An $R \times C$ grid can be modeled as a graph with $N = R \times C$ vertices, where each 0-indexed coordinate $(r, c)$ is mapped bijectively to a 1D integer index via **coordinate flattening**:
$$\text{id}(r, c) = r \cdot C + c, \quad 0 \le r < R, \; 0 \le c < C$$

1. **Incremental Component Counting:** Initially, the grid has $0$ land cells and $\text{count} = 0$ islands.
2. **Adding a Land Cell at $(r, c)$:**
   - If `is_land[id(r, c)]` is already `true` (duplicate coordinate), the grid state is unchanged and we return $\text{count}$.
   - Otherwise, we set `is_land[id(r, c)] = true` and tentatively treat the new cell as an isolated island by incrementing $\text{count} \leftarrow \text{count} + 1$.
3. **Merging 4-Directional Neighbors:** We inspect the four orthogonal neighbors $(r \pm 1, c)$ and $(r, c \pm 1)$ that lie within grid boundaries. For each neighbor $(n_r, n_c)$ that is already land (`is_land[id(nr, nc)] == true`), we check whether $\text{findSet}(\text{id}(r, c)) \neq \text{findSet}(\text{id}(n_r, n_c))$. If they belong to different DSU components, we merge them via union by rank and decrement the number of islands:
   $$\text{count} \leftarrow \text{count} - 1$$

## Complexity Analysis
- **Time complexity:** $\mathcal{O}(R \cdot C + k \cdot \alpha(R \cdot C))$, where $\mathcal{O}(R \cdot C)$ is the one-time DSU initialization cost and each of the $k$ land addition operations performs at most $4$ `findSet`/`unionSets` calls in $\mathcal{O}(\alpha(R \cdot C))$ amortized time.
- **Extra space complexity:** $\mathcal{O}(R \cdot C)$ auxiliary space to store the `parent`, `rank`, and `is_land` arrays for the $R \times C$ grid cells.
