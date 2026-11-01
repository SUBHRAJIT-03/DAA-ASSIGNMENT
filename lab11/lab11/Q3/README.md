# Q03 - Bidirectional BFS for Shortest Transformation Sequence in a Word Graph

## Question Title
**[Bidirectional BFS for Shortest Transformation Sequence in a Word Graph]** You are given:
- A dictionary $D$ containing $N$ unique words, where every word has the exact same length $L$.
- A starting word $\mathbf{A}$ and a target word $\mathbf{B}$, both of length $L$.

A *valid mutation step* consists of changing exactly one character of a word such that the resulting word exists in dictionary $D$. A **transformation sequence** from $\mathbf{A}$ to $\mathbf{B}$ is an ordered sequence of words:
$$W_0, W_1, W_2, \dots, W_k$$
such that:
1. $W_0 = \mathbf{A}$ and $W_k = \mathbf{B}$.
2. Every intermediate word $W_i \in D$ for all $1 \le i \le k$. (Note: whether $\mathbf{A}$ must initially be in $D$ is typically specified by the input constraints, but $\mathbf{B}$ must always be in $D$.)
3. For every adjacent pair $(W_{i-1}, W_i)$, the two words differ by exactly one character (i.e., their Hamming distance is $1$).

*Objective:* Compute the minimum number of mutations $k$ required to transform $\mathbf{A}$ into $\mathbf{B}$. If $\mathbf{A}$ cannot be transformed into $\mathbf{B}$ through valid intermediate words in $D$, return $-1$ (or report that no valid path exists). Keep in mind to consider the following constraints for this problem during implementation:
- $N \le 10^5$ (number of dictionary words)
- $L \le 10$ (length of each word)
- All words consist solely of lowercase English letters (`'a'` to `'z'`).

By choosing the proper input representation, write a program in C to validate your algorithm and derive the complexity analysis of your algorithm.

## How the Code Works
The program offers a menu-driven choice between manual entry and constraint-compliant randomized dictionary generation ($N \le 10^5, L \le 10$, lowercase English letters), indexing all dictionary words in an open-addressed FNV-1a hash table for $\mathcal{O}(L)$ lookup. After verifying that target word $\mathbf{B} \in D$, it launches a **Bidirectional BFS** from both $\mathbf{A}$ (forward frontier) and $\mathbf{B}$ (backward frontier), always expanding the smaller frontier one level at a time via single-character mutations. When the two frontiers intersect at a common word, it outputs the minimum mutation count $k = \text{dist}_{\text{fwd}}[u] + \text{dist}_{\text{bwd}}[u]$ along with the reconstructed transformation sequence, or `-1` if no path exists.

## Maths / Logic Behind This
We model the dictionary as an implicit unweighted, undirected graph $G = (V, E)$ where $V = D \cup \{\mathbf{A}\}$ and an edge $(u, v) \in E$ exists if and only if the Hamming distance $d_H(u, v) = 1$.

1. **Implicit Neighborhood Generation via Hashing**: Explicitly constructing $E$ by comparing all $\binom{N}{2}$ word pairs would take $\mathcal{O}(N^2 L)$ time, which is infeasible for $N = 10^5$ ($\sim 10^{10}$ operations). Instead, each word of length $L$ has at most $25L$ possible single-character mutations. By storing $D$ in an open-addressed hash table of capacity $2^{18} > 2N$, testing each candidate mutation takes $\mathcal{O}(L)$ expected time, so enumerating all neighbors of a word takes $\mathcal{O}(26 L^2)$ time.
2. **Bidirectional BFS Frontier Reduction**: A standard unidirectional BFS with branching factor $b$ and shortest path length $k$ explores $\mathcal{O}(b^k)$ states. By simultaneously searching forward from $\mathbf{A}$ and backward from $\mathbf{B}$ and expanding the smaller frontier at each step, both searches meet near depth $k/2$, reducing the explored state space from $\mathcal{O}(b^k)$ to $\mathcal{O}(b^{k/2})$ while guaranteeing optimality on unweighted graphs.

## Complexity Analysis
- Time complexity: $\mathcal{O}(N \cdot L^2)$ worst-case expected time (more tighter in practice as $\mathcal{O}(b^{k/2} \cdot 26 L^2)$ where $k$ is the shortest path length), since each of the $N$ words is inserted into the hash table in $\mathcal{O}(L)$ and expanded at most once across both frontiers with $25L$ hash lookups of cost $\mathcal{O}(L)$ each.
- Extra space complexity: $\mathcal{O}(N \cdot L)$ to store the $N$ dictionary strings of length $L$, the open-addressed hash table, and the bidirectional BFS queues, distance arrays, and parent pointers of size $\mathcal{O}(N)$.
