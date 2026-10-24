#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* Pairwise swap edge between 1-indexed positions u and v */
typedef struct {
    int u;
    int v;
} SwapEdge;

/* Disjoint Set Union (DSU) for grouping swappable permutation indices */
typedef struct {
    int *parent;
    int *rank;
    int n;
} DSU;

/* Function Prototypes */
DSU* createDSU(int n);
void freeDSU(DSU *dsu);
int findSet(DSU *dsu, int x);
void unionSets(DSU *dsu, int x, int y);
int compareIntsAsc(const void *a, const void *b);
void computeSmallestPermutation(int n, int m, const int *A, const SwapEdge *swaps);
void handleManualInput(int *n, int *m, int **A, SwapEdge **swaps);
void generateRandomData(int *n, int *m, int **A, SwapEdge **swaps);

/* Initialize DSU for n indices (1-indexed: 1 to n) */
DSU* createDSU(int n) {
    DSU *dsu = (DSU *)malloc(sizeof(DSU));
    dsu->n = n;
    dsu->parent = (int *)malloc((n + 1) * sizeof(int));
    dsu->rank = (int *)calloc((n + 1), sizeof(int));

    for (int i = 1; i <= n; i++) {
        dsu->parent[i] = i;
    }
    return dsu;
}

/* Free allocated DSU memory */
void freeDSU(DSU *dsu) {
    if (dsu) {
        free(dsu->parent);
        free(dsu->rank);
        free(dsu);
    }
}

/* Find component root with path compression */
int findSet(DSU *dsu, int x) {
    if (dsu->parent[x] != x) {
        dsu->parent[x] = findSet(dsu, dsu->parent[x]);
    }
    return dsu->parent[x];
}

/* Union two sets by rank */
void unionSets(DSU *dsu, int x, int y) {
    int root_x = findSet(dsu, x);
    int root_y = findSet(dsu, y);

    if (root_x == root_y) {
        return;
    }

    if (dsu->rank[root_x] < dsu->rank[root_y]) {
        dsu->parent[root_x] = root_y;
    } else if (dsu->rank[root_x] > dsu->rank[root_y]) {
        dsu->parent[root_y] = root_x;
    } else {
        dsu->parent[root_y] = root_x;
        dsu->rank[root_x]++;
    }
}

/* Comparator for sorting integers in ascending order */
int compareIntsAsc(const void *a, const void *b) {
    int val_a = *(const int *)a;
    int val_b = *(const int *)b;
    return (val_a > val_b) - (val_a < val_b);
}

/*
 * Find the lexicographically smallest permutation achievable via any number of
 * allowable pairwise swaps by grouping indices into connected components and
 * sorting the values within each component.
 */
void computeSmallestPermutation(int n, int m, const int *A, const SwapEdge *swaps) {
    DSU *dsu = createDSU(n);

    for (int i = 0; i < m; i++) {
        if (swaps[i].u >= 1 && swaps[i].u <= n && swaps[i].v >= 1 && swaps[i].v <= n) {
            unionSets(dsu, swaps[i].u, swaps[i].v);
        }
    }

    /* Count size of each connected component */
    int *comp_size = (int *)calloc(n + 1, sizeof(int));
    for (int i = 1; i <= n; i++) {
        comp_size[findSet(dsu, i)]++;
    }

    /* Allocate CSR-style buckets to group indices and values by component root */
    int *comp_offset = (int *)calloc(n + 2, sizeof(int));
    for (int r = 1; r <= n; r++) {
        comp_offset[r + 1] = comp_offset[r] + comp_size[r];
    }

    int *write_pos = (int *)malloc((n + 1) * sizeof(int));
    for (int r = 1; r <= n; r++) {
        write_pos[r] = comp_offset[r];
    }

    int *grouped_indices = (int *)malloc(n * sizeof(int));
    int *grouped_values = (int *)malloc(n * sizeof(int));

    /* Iterate i = 1..n in ascending order so grouped_indices is naturally sorted per component */
    for (int i = 1; i <= n; i++) {
        int r = findSet(dsu, i);
        int pos = write_pos[r]++;
        grouped_indices[pos] = i;
        grouped_values[pos] = A[i];
    }

    /* Sort the values within each component in ascending order and place them at the component's indices */
    int *result = (int *)malloc((n + 1) * sizeof(int));
    for (int r = 1; r <= n; r++) {
        int count = comp_size[r];
        if (count > 0) {
            int start = comp_offset[r];
            qsort(&grouped_values[start], count, sizeof(int), compareIntsAsc);
            for (int k = 0; k < count; k++) {
                result[grouped_indices[start + k]] = grouped_values[start + k];
            }
        }
    }

    printf("\n--- Lexicographically Smallest Permutation Result ---\n");
    printf("Initial Permutation A : [");
    for (int i = 1; i <= n; i++) {
        printf("%d%s", A[i], (i == n) ? "]\n" : ", ");
    }

    printf("Smallest Permutation  : [");
    for (int i = 1; i <= n; i++) {
        printf("%d%s", result[i], (i == n) ? "]\n" : ", ");
    }

    free(comp_size);
    free(comp_offset);
    free(write_pos);
    free(grouped_indices);
    free(grouped_values);
    free(result);
    freeDSU(dsu);
}

/* Handle manual user input */
void handleManualInput(int *n, int *m, int **A, SwapEdge **swaps) {
    printf("Enter permutation size (n >= 1) and number of allowable swap pairs (m >= 0): ");
    if (scanf("%d %d", n, m) != 2 || *n < 1 || *m < 0) {
        fprintf(stderr, "Invalid n or m.\n");
        exit(EXIT_FAILURE);
    }

    *A = (int *)malloc((*n + 1) * sizeof(int));
    bool *seen = (bool *)calloc(*n + 1, sizeof(bool));
    printf("Enter the permutation A of numbers 1..%d:\n", *n);
    for (int i = 1; i <= *n; i++) {
        if (scanf("%d", &((*A)[i])) != 1 || (*A)[i] < 1 || (*A)[i] > *n || seen[(*A)[i]]) {
            fprintf(stderr, "Invalid input: A must be a valid permutation of 1..%d.\n", *n);
            free(seen);
            free(*A);
            exit(EXIT_FAILURE);
        }
        seen[(*A)[i]] = true;
    }
    free(seen);

    *swaps = (SwapEdge *)malloc(((*m > 0) ? *m : 1) * sizeof(SwapEdge));
    if (*m > 0) {
        printf("Enter %d allowable pairwise swap edges 'u v' (1-indexed positions 1 to %d):\n", *m, *n);
        for (int i = 0; i < *m; i++) {
            printf("Swap %d: ", i + 1);
            if (scanf("%d %d", &((*swaps)[i].u), &((*swaps)[i].v)) != 2) {
                fprintf(stderr, "Invalid swap pair input.\n");
                free(*A);
                free(*swaps);
                exit(EXIT_FAILURE);
            }
        }
    }
}

/* Generate a random permutation of 1..n and random swap edges */
void generateRandomData(int *n, int *m, int **A, SwapEdge **swaps) {
    srand((unsigned int)time(NULL));
    *n = 7 + (rand() % 4);        /* Permutation length 7 to 10 */
    *m = (*n / 2) + (rand() % 4); /* Number of allowable swap edges */

    *A = (int *)malloc((*n + 1) * sizeof(int));
    for (int i = 1; i <= *n; i++) {
        (*A)[i] = i;
    }
    /* Fisher-Yates shuffle to produce a random permutation of 1..n */
    for (int i = *n; i >= 2; i--) {
        int j = 1 + (rand() % i);
        int temp = (*A)[i];
        (*A)[i] = (*A)[j];
        (*A)[j] = temp;
    }

    *swaps = (SwapEdge *)malloc((*m) * sizeof(SwapEdge));
    printf("\nGenerated Parameters: n = %d, m = %d allowable swaps.\n", *n, *m);
    printf("Generated Allowable Swap Edges (1-indexed positions): ");
    for (int i = 0; i < *m; i++) {
        int u = 1 + (rand() % (*n));
        int v = 1 + (rand() % (*n));
        while (v == u && *n > 1) {
            v = 1 + (rand() % (*n));
        }
        (*swaps)[i].u = u;
        (*swaps)[i].v = v;
        printf("(%d, %d)%s", u, v, (i == *m - 1) ? "\n" : ", ");
    }
}

int main(void) {
    int choice;
    int n = 0, m = 0;
    int *A = NULL;
    SwapEdge *swaps = NULL;

    printf("=============================================================\n");
    printf(" Q05: Lexicographically Smallest Connected Component Perm.\n");
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
        handleManualInput(&n, &m, &A, &swaps);
    } else if (choice == 2) {
        generateRandomData(&n, &m, &A, &swaps);
    } else {
        fprintf(stderr, "Invalid choice. Please select 1 or 2.\n");
        return EXIT_FAILURE;
    }

    computeSmallestPermutation(n, m, A, swaps);

    free(A);
    free(swaps);
    return EXIT_SUCCESS;
}
