#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* Adjacency list representation for an undirected graph */
typedef struct {
    int *head;
    int *to;
    int *next;
    int edge_count;
    int num_vertices;
} Graph;

/* Disjoint Set Union tracking component vertex weight sums */
typedef struct {
    int *parent;
    int *rank;
    long long *comp_weight;
    int num_vertices;
} ReverseDSU;

/* Function Prototypes */
Graph* createGraph(int V, int max_edges);
void addGraphEdge(Graph *g, int u, int v);
void freeGraph(Graph *g);
ReverseDSU* createReverseDSU(int V, const long long *weights);
void freeReverseDSU(ReverseDSU *dsu);
int findSet(ReverseDSU *dsu, int v);
long long unionSets(ReverseDSU *dsu, int u, int v);
void processReverseDeletions(int V, const long long *weights, Graph *g, const int *deletion_order);
void handleManualInput(int *V, int *E, long long **weights, Graph **g, int **deletion_order);
void generateRandomData(int *V, int *E, long long **weights, Graph **g, int **deletion_order);

/* Allocate adjacency list for V vertices and up to max_edges undirected edges */
Graph* createGraph(int V, int max_edges) {
    Graph *g = (Graph *)malloc(sizeof(Graph));
    g->num_vertices = V;
    g->edge_count = 0;
    g->head = (int *)malloc((V + 1) * sizeof(int));
    g->to = (int *)malloc((2 * max_edges + 2) * sizeof(int));
    g->next = (int *)malloc((2 * max_edges + 2) * sizeof(int));

    for (int i = 1; i <= V; i++) {
        g->head[i] = -1;
    }
    return g;
}

/* Add an undirected edge (u, v) to the adjacency list */
void addGraphEdge(Graph *g, int u, int v) {
    g->to[g->edge_count] = v;
    g->next[g->edge_count] = g->head[u];
    g->head[u] = g->edge_count++;

    g->to[g->edge_count] = u;
    g->next[g->edge_count] = g->head[v];
    g->head[v] = g->edge_count++;
}

/* Free graph memory */
void freeGraph(Graph *g) {
    if (g) {
        free(g->head);
        free(g->to);
        free(g->next);
        free(g);
    }
}

/* Initialize ReverseDSU for V vertices */
ReverseDSU* createReverseDSU(int V, const long long *weights) {
    ReverseDSU *dsu = (ReverseDSU *)malloc(sizeof(ReverseDSU));
    dsu->num_vertices = V;
    dsu->parent = (int *)malloc((V + 1) * sizeof(int));
    dsu->rank = (int *)calloc((V + 1), sizeof(int));
    dsu->comp_weight = (long long *)malloc((V + 1) * sizeof(long long));

    for (int i = 1; i <= V; i++) {
        dsu->parent[i] = i;
        dsu->comp_weight[i] = weights[i];
    }
    return dsu;
}

/* Free ReverseDSU memory */
void freeReverseDSU(ReverseDSU *dsu) {
    if (dsu) {
        free(dsu->parent);
        free(dsu->rank);
        free(dsu->comp_weight);
        free(dsu);
    }
}

/* Find component representative with path compression */
int findSet(ReverseDSU *dsu, int v) {
    if (dsu->parent[v] != v) {
        dsu->parent[v] = findSet(dsu, dsu->parent[v]);
    }
    return dsu->parent[v];
}

/* Union two sets by rank and return the merged component's total vertex weight */
long long unionSets(ReverseDSU *dsu, int u, int v) {
    int root_u = findSet(dsu, u);
    int root_v = findSet(dsu, v);

    if (root_u == root_v) {
        return dsu->comp_weight[root_u];
    }

    if (dsu->rank[root_u] < dsu->rank[root_v]) {
        dsu->parent[root_u] = root_v;
        dsu->comp_weight[root_v] += dsu->comp_weight[root_u];
        return dsu->comp_weight[root_v];
    } else if (dsu->rank[root_u] > dsu->rank[root_v]) {
        dsu->parent[root_v] = root_u;
        dsu->comp_weight[root_u] += dsu->comp_weight[root_v];
        return dsu->comp_weight[root_u];
    } else {
        dsu->parent[root_v] = root_u;
        dsu->comp_weight[root_u] += dsu->comp_weight[root_v];
        dsu->rank[root_u]++;
        return dsu->comp_weight[root_u];
    }
}

/*
 * Solve sequential vertex deletions by reversing time:
 * Start from an empty graph and add vertices back in reverse deletion order
 * P[V], P[V-1], ..., P[1], maintaining the running maximum component sum.
 */
void processReverseDeletions(int V, const long long *weights, Graph *g, const int *deletion_order) {
    ReverseDSU *dsu = createReverseDSU(V, weights);
    bool *active = (bool *)calloc(V + 1, sizeof(bool));
    long long *ans_after_deletion = (long long *)calloc(V + 1, sizeof(long long));

    long long current_max_sum = 0;

    /* After the last vertex P[V] is deleted, no vertices remain (sum = 0) */
    ans_after_deletion[V] = 0;

    /* Traverse deletion sequence backwards from step V down to 1 */
    for (int step = V; step >= 1; step--) {
        /* Record the max component sum right before P[step] is added (i.e. after P[step] is deleted) */
        ans_after_deletion[step] = current_max_sum;

        int u = deletion_order[step];
        active[u] = true;

        long long u_comp_weight = dsu->comp_weight[findSet(dsu, u)];
        if (u_comp_weight > current_max_sum) {
            current_max_sum = u_comp_weight;
        }

        /* Connect u to all adjacent vertices v that are already active */
        for (int e = g->head[u]; e != -1; e = g->next[e]) {
            int v = g->to[e];
            if (active[v]) {
                long long merged_weight = unionSets(dsu, u, v);
                if (merged_weight > current_max_sum) {
                    current_max_sum = merged_weight;
                }
            }
        }
    }

    printf("\n--- Reverse Time-Step DSU Vertex Deletion Log ---\n");
    printf("Initial Graph (before any deletion) -> Max Component Vertex Sum = %lld\n", current_max_sum);
    for (int step = 1; step <= V; step++) {
        printf("After deleting vertex %2d (Step %2d/%2d) -> Max Remaining Component Sum = %lld\n",
               deletion_order[step], step, V, ans_after_deletion[step]);
    }

    free(active);
    free(ans_after_deletion);
    freeReverseDSU(dsu);
}

/* Handle manual user input */
void handleManualInput(int *V, int *E, long long **weights, Graph **g, int **deletion_order) {
    printf("Enter number of vertices (V >= 1) and number of edges (E >= 0): ");
    if (scanf("%d %d", V, E) != 2 || *V < 1 || *E < 0) {
        fprintf(stderr, "Invalid V or E.\n");
        exit(EXIT_FAILURE);
    }

    *weights = (long long *)malloc((*V + 1) * sizeof(long long));
    printf("Enter the %d positive vertex weights (w_1 to w_%d):\n", *V, *V);
    for (int i = 1; i <= *V; i++) {
        if (scanf("%lld", &((*weights)[i])) != 1) {
            fprintf(stderr, "Invalid weight input.\n");
            free(*weights);
            exit(EXIT_FAILURE);
        }
    }

    *g = createGraph(*V, *E);
    if (*E > 0) {
        printf("Enter %d undirected edges as 'u v' (1-indexed vertices 1 to %d):\n", *E, *V);
        for (int i = 0; i < *E; i++) {
            int u, v;
            printf("Edge %d: ", i + 1);
            if (scanf("%d %d", &u, &v) != 2 || u < 1 || u > *V || v < 1 || v > *V) {
                fprintf(stderr, "Invalid edge endpoints.\n");
                free(*weights);
                freeGraph(*g);
                exit(EXIT_FAILURE);
            }
            addGraphEdge(*g, u, v);
        }
    }

    *deletion_order = (int *)malloc((*V + 1) * sizeof(int));
    bool *seen = (bool *)calloc(*V + 1, sizeof(bool));
    printf("Enter the deletion permutation P = [p_1, p_2, ..., p_%d]:\n", *V);
    for (int i = 1; i <= *V; i++) {
        if (scanf("%d", &((*deletion_order)[i])) != 1 ||
            (*deletion_order)[i] < 1 || (*deletion_order)[i] > *V || seen[(*deletion_order)[i]]) {
            fprintf(stderr, "Invalid deletion permutation (must be distinct vertices in 1..%d).\n", *V);
            free(seen);
            free(*weights);
            freeGraph(*g);
            free(*deletion_order);
            exit(EXIT_FAILURE);
        }
        seen[(*deletion_order)[i]] = true;
    }
    free(seen);
}

/* Generate randomized vertex-weighted graph and deletion permutation P */
void generateRandomData(int *V, int *E, long long **weights, Graph **g, int **deletion_order) {
    srand((unsigned int)time(NULL));
    *V = 6 + (rand() % 4);        /* 6 to 9 vertices */
    *E = *V + (rand() % (*V));    /* V to 2V-1 edges */

    *weights = (long long *)malloc((*V + 1) * sizeof(long long));
    printf("\nGenerated Parameters: V = %d vertices, E = %d edges.\n", *V, *E);
    printf("Generated Vertex Weights: ");
    for (int i = 1; i <= *V; i++) {
        (*weights)[i] = 5 + (rand() % 45); /* 5 to 49 */
        printf("w[%d]=%lld%s", i, (*weights)[i], (i == *V) ? "\n" : ", ");
    }

    *g = createGraph(*V, *E);
    printf("Generated Undirected Edges: ");
    for (int i = 0; i < *E; i++) {
        int u = 1 + (rand() % (*V));
        int v = 1 + (rand() % (*V));
        while (v == u && *V > 1) {
            v = 1 + (rand() % (*V));
        }
        addGraphEdge(*g, u, v);
        printf("(%d, %d)%s", u, v, (i == *E - 1) ? "\n" : ", ");
    }

    /* Generate a random permutation P of {1, 2, ..., V} using Fisher-Yates shuffle */
    *deletion_order = (int *)malloc((*V + 1) * sizeof(int));
    for (int i = 1; i <= *V; i++) {
        (*deletion_order)[i] = i;
    }
    for (int i = *V; i >= 2; i--) {
        int j = 1 + (rand() % i);
        int temp = (*deletion_order)[i];
        (*deletion_order)[i] = (*deletion_order)[j];
        (*deletion_order)[j] = temp;
    }

    printf("Generated Deletion Order P: [");
    for (int i = 1; i <= *V; i++) {
        printf("%d%s", (*deletion_order)[i], (i == *V) ? "]\n" : ", ");
    }
}

int main(void) {
    int choice;
    int V = 0, E = 0;
    long long *weights = NULL;
    Graph *g = NULL;
    int *deletion_order = NULL;

    printf("=============================================================\n");
    printf(" Q04: Reverse Time-Step DSU Application (Vertex Deletions)\n");
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
        handleManualInput(&V, &E, &weights, &g, &deletion_order);
    } else if (choice == 2) {
        generateRandomData(&V, &E, &weights, &g, &deletion_order);
    } else {
        fprintf(stderr, "Invalid choice. Please select 1 or 2.\n");
        return EXIT_FAILURE;
    }

    processReverseDeletions(V, weights, g, deletion_order);

    free(weights);
    freeGraph(g);
    free(deletion_order);
    return EXIT_SUCCESS;
}
