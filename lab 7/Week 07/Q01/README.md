## Q01 - Invert the Coin Triangle

## How the Code Works
The program first asks whether the user wants to enter the value of `n` manually or generate a random value. Once `n` is chosen, it builds the triangular arrangement of coins as rows of increasing length. Each coin is represented by integer coordinates derived from its row and column.

The code then computes the centroid of all coin positions and rotates every point by 180 degrees about that centroid. The key idea is to compare the rotated arrangement with the original arrangement to count how many coins already match their target positions. The minimum number of moves is found by subtracting this overlap count from the total number of coins.

## Maths / Logic Behind This
For a triangular arrangement of side length `n`, the total number of coins is:

T = n(n + 1) / 2

Each coin is mapped to coordinates:

X = 2c - r
Y = r

where `r` is the row number and `c` is the column number. After a 180-degree rotation about the centroid `C`, a point transforms as:

p' = 2C - p

The minimum slide count is:

Minimum moves = T - overlap

where `overlap` is how many positions remain unchanged after the rotation.

## Complexity Analysis
- Time Complexity: O(T log T), because the coin positions are sorted before checking overlap.
- Space Complexity: O(T), because the program stores all coin coordinates.

Here, `T = n(n + 1) / 2` is the total number of coins.

## Output
The program prints:
- the chosen value of `n`
- total number of coins `T`
- overlap count after rotation
- minimum moves required to invert the triangle
