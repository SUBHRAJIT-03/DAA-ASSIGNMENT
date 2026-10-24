# Q05 - Lexicographically Smallest Connected Component Permutation

## Question Title
[Lexicographically Smallest Connected Component Permutation] Given a permutation $A$ of numbers $1 \dots n$, and a list of allowable pairwise swaps $(u, v)$ forming edges in an undirected graph, find the lexicographically smallest permutation achievable through any number of valid swaps. By choosing the proper input representation, write a program in C to validate your algorithm and derive the complexity analysis of your algorithm.

## How the Code Works
The program starts with a menu allowing the user to either enter the permutation $A$ and swap pairs manually (`handleManualInput`) or generate a random permutation and swap edges (`generateRandomData`). In `computeSmallestPermutation`, a Disjoint Set Union (`DSU`) groups all indices $1 \dots n$ connected by allowable swap edges into connected components. Using compressed flat buckets, the program collects the indices and their corresponding values for each component, sorts the values in ascending order via `qsort`, and writes them back into the component's ascending indices to form the lexicographically smallest permutation.

## Maths / Logic Behind This
1. **Symmetric Group Generation on Connected Components:** Every allowable swap $(u, v)$ corresponds to a transposition $(u \; v)$ on the indices of permutation $A$. A fundamental result in group theory states that the transpositions corresponding to the edges of any connected undirected graph $C_r$ generate the full **symmetric group** $S_{|C_r|}$ on the vertex set of $C_r$. Consequently, elements located at indices belonging to the same connected component can be rearranged into *any* relative order among those indices, while no element can ever move to an index outside its connected component.
2. **Greedy Lexicographical Optimality:** Since the connected components partition the index set $\{1, 2, \dots, n\}$ into disjoint equivalence classes $C_1, C_2, \dots, C_k$, minimizing the permutation lexicographically requires placing the smallest available value in each component at the earliest available index in that component.
3. **Algorithmic Steps:**
   - Process all $m$ swap pairs $(u, v)$ using DSU (`unionSets(u, v)`).
   - Partition the index set $\{1, \dots, n\}$ by component root $r = \text{findSet}(i)$, collecting the sorted index list $I_r = (i_1 < i_2 < \dots < i_s)$ and value multiset $V_r = \{A[i_1], A[i_2], \dots, A[i_s]\}$.
   - Sort $V_r$ in ascending order $(v_1 < v_2 < \dots < v_s)$ and assign $A'[i_j] = v_j$ for all $1 \le j \le s$.

## Complexity Analysis
- **Time complexity:** $\mathcal{O}(m \cdot \alpha(n) + n \log n)$, where $\mathcal{O}(m \cdot \alpha(n))$ is the time to process $m$ swap edges in the DSU and $\mathcal{O}\left(\sum_{r} |C_r| \log |C_r|\right) = \mathcal{O}(n \log n)$ is the total time to sort the values across all disjoint components.
- **Extra space complexity:** $\mathcal{O}(n)$ auxiliary space to store the DSU arrays (`parent`, `rank`), component offset tables, and grouped index/value buckets.
