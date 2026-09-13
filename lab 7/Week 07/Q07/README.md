## Q07 - Matrix Chain Multiplication

## How the Code Works
The program first asks whether the user wants to enter the matrix dimensions manually or generate a random set of dimensions. A valid matrix chain is represented by an array `p`, where `p[0]` to `p[n]` are the dimensions of the chain.

The program fills a dynamic programming table `m[i][j]` representing the minimum number of scalar multiplications required to multiply matrices from index `i` to `j`. It then tries every possible split point `k` and keeps the minimum value. The split location is stored in another table, which is used later to reconstruct the optimal parenthesization.

## Maths / Logic Behind This
For a matrix chain with dimensions `p[i-1] × p[i]`, the cost of splitting at `k` is:

m[i][j] = min over k of (m[i][k] + m[k+1][j] + p[i-1] × p[k] × p[j])

The base case is:

m[i][i] = 0

This means multiplying a single matrix costs zero scalar multiplications. The algorithm computes the minimal total multiplication cost for the whole chain and reconstructs the optimal grouping order.

## Complexity Analysis
- Time Complexity: O(n³)
- Space Complexity: O(n²)

The DP table has size `n × n`, and every interval is evaluated over all possible split points.

## Output
The program prints:
- the minimum number of scalar multiplications required
- the optimal parenthesization of the matrix chain
