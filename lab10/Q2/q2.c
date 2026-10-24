#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* Coordinate pair representing a grid cell (0-indexed: 0 <= r < R, 0 <= c < C) */
typedef struct {
    int r;
    int c;
} Cell;

/* Disjoint Set Union (DSU) for flattened 2D grid coordinates */
typedef struct {
    int *parent;
    int *rank;
    bool *is_land;
    int rows;
    int cols;
    int island_count;
} GridDSU;

/* Function Prototypes */
GridDSU* createGridDSU(int R, int C);
void freeGridDSU(GridDSU *dsu);
int flattenCoord(int r, int c, int C);
int findSet(GridDSU *dsu, int idx);
bool unionSets(GridDSU *dsu, int idx1, int idx2);
int addLandCell(GridDSU *dsu, int r, int c);
void simulateGridOperations(int R, int C, int K, Cell *operations);
void handleManualInput(int *R, int *C, int *K, Cell **operations);
void generateRandomData(int *R, int *C, int *K, Cell **operations);

/* Initialize GridDSU for an R x C grid initially filled with water */
GridDSU* createGridDSU(int R, int C) {
    int total_cells = R * C;
    GridDSU *dsu = (GridDSU *)malloc(sizeof(GridDSU));
    dsu->rows = R;
    dsu->cols = C;
    dsu->island_count = 0;
    dsu->parent = (int *)malloc(total_cells * sizeof(int));
    dsu->rank = (int *)calloc(total_cells, sizeof(int));
    dsu->is_land = (bool *)calloc(total_cells, sizeof(bool));

    for (int i = 0; i < total_cells; i++) {
        dsu->parent[i] = i;
    }
    return dsu;
}

/* Free allocated GridDSU memory */
void freeGridDSU(GridDSU *dsu) {
    if (dsu) {
        free(dsu->parent);
        free(dsu->rank);
        free(dsu->is_land);
        free(dsu);
    }
}

/* Flatten 2D coordinate (r, c) into 1D index */
int flattenCoord(int r, int c, int C) {
    return r * C + c;
}

/* Find set representative with path compression */
int findSet(GridDSU *dsu, int idx) {
    if (dsu->parent[idx] != idx) {
        dsu->parent[idx] = findSet(dsu, dsu->parent[idx]);
    }
    return dsu->parent[idx];
}

/* Union two sets by rank; returns true if two distinct components were merged */
bool unionSets(GridDSU *dsu, int idx1, int idx2) {
    int root1 = findSet(dsu, idx1);
    int root2 = findSet(dsu, idx2);

    if (root1 == root2) {
        return false;
    }

    if (dsu->rank[root1] < dsu->rank[root2]) {
        dsu->parent[root1] = root2;
    } else if (dsu->rank[root1] > dsu->rank[root2]) {
        dsu->parent[root2] = root1;
    } else {
        dsu->parent[root2] = root1;
        dsu->rank[root1]++;
    }
    return true;
}

/* Add a land cell at (r, c) and return the updated number of connected islands */
int addLandCell(GridDSU *dsu, int r, int c) {
    int idx = flattenCoord(r, c, dsu->cols);

    /* If cell is already land, number of islands does not change */
    if (dsu->is_land[idx]) {
        return dsu->island_count;
    }

    dsu->is_land[idx] = true;
    dsu->island_count++;

    /* 4-directional movement deltas: Up, Down, Left, Right */
    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    for (int d = 0; d < 4; d++) {
        int nr = r + dr[d];
        int nc = c + dc[d];

        if (nr >= 0 && nr < dsu->rows && nc >= 0 && nc < dsu->cols) {
            int n_idx = flattenCoord(nr, nc, dsu->cols);
            if (dsu->is_land[n_idx]) {
                if (unionSets(dsu, idx, n_idx)) {
                    dsu->island_count--;
                }
            }
        }
    }

    return dsu->island_count;
}

/* Execute all K land-addition operations and output island count after each */
void simulateGridOperations(int R, int C, int K, Cell *operations) {
    GridDSU *dsu = createGridDSU(R, C);

    printf("\n--- Dynamic Connected Components (Islands) Log ---\n");
    for (int i = 0; i < K; i++) {
        int r = operations[i].r;
        int c = operations[i].c;

        if (r < 0 || r >= R || c < 0 || c >= C) {
            printf("Operation %2d: Add Land (%d, %d) -> OUT OF BOUNDS | Current Islands = %d\n",
                   i + 1, r, c, dsu->island_count);
            continue;
        }

        int current_islands = addLandCell(dsu, r, c);
        printf("Operation %2d: Add Land (%d, %d) -> Current Number of Islands = %d\n",
               i + 1, r, c, current_islands);
    }

    freeGridDSU(dsu);
}

/* Handle manual user input */
void handleManualInput(int *R, int *C, int *K, Cell **operations) {
    printf("Enter grid dimensions R (rows >= 1) and C (cols >= 1): ");
    if (scanf("%d %d", R, C) != 2 || *R < 1 || *C < 1) {
        fprintf(stderr, "Invalid grid dimensions.\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter number of land addition operations (k >= 1): ");
    if (scanf("%d", K) != 1 || *K < 1) {
        fprintf(stderr, "Invalid number of operations.\n");
        exit(EXIT_FAILURE);
    }

    *operations = (Cell *)malloc((*K) * sizeof(Cell));
    printf("Enter %d land coordinates as 'r c' (0-indexed: 0 <= r < %d, 0 <= c < %d):\n", *K, *R, *C);
    for (int i = 0; i < *K; i++) {
        printf("Operation %d: ", i + 1);
        if (scanf("%d %d", &((*operations)[i].r), &((*operations)[i].c)) != 2) {
            fprintf(stderr, "Invalid coordinate input.\n");
            free(*operations);
            exit(EXIT_FAILURE);
        }
    }
}

/* Generate randomized grid and land addition operations */
void generateRandomData(int *R, int *C, int *K, Cell **operations) {
    srand((unsigned int)time(NULL));
    *R = 4 + (rand() % 3); /* 4 to 6 rows */
    *C = 4 + (rand() % 3); /* 4 to 6 cols */
    *K = 8 + (rand() % 5); /* 8 to 12 operations */

    *operations = (Cell *)malloc((*K) * sizeof(Cell));
    printf("\nGenerated Random Grid: R = %d, C = %d, k = %d operations.\n", *R, *C, *K);
    printf("Generated Land Positions (0-indexed): ");
    for (int i = 0; i < *K; i++) {
        (*operations)[i].r = rand() % (*R);
        (*operations)[i].c = rand() % (*C);
        printf("(%d, %d)%s", (*operations)[i].r, (*operations)[i].c, (i == *K - 1) ? "\n" : ", ");
    }
}

int main(void) {
    int choice;
    int R = 0, C = 0, K = 0;
    Cell *operations = NULL;

    printf("=============================================================\n");
    printf(" Q02: Dynamic Connected Components in a Grid (Islands II)\n");
    printf("=============================================================\n");
    printf("Select Input Method:\n");
    printf("  1. Manual User Input\n");
    printf("  2. Randomized Data Generation\n");
    printf("Enter your choice (1 or 2): ");

    if (scanf("%d", &choice) != 1) {
        fprintf(stderr, "Invalid menu selection.\n");
        return EXIT_FAILURE;
    }

    if (choice == 1) {
        handleManualInput(&R, &C, &K, &operations);
    } else if (choice == 2) {
        generateRandomData(&R, &C, &K, &operations);
    } else {
        fprintf(stderr, "Invalid choice. Please select 1 or 2.\n");
        return EXIT_FAILURE;
    }

    simulateGridOperations(R, C, K, operations);

    free(operations);
    return EXIT_SUCCESS;
}
