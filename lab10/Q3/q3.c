#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>

/* Structure representing a weighted undirected edge arriving sequentially */
typedef struct {
    int u;
    int v;
    long long edge_weight;
} WeightedEdge;

/* Disjoint Set Union tracking component vertex size and total vertex weight */
typedef struct {
    int *parent;
    int *rank;
    int *comp_size;
    long long *comp_vertex_weight;
    long long *comp_edge_weight;
    int num_vertices;
} WeightedDSU;

/* Function Prototypes */
WeightedDSU* createWeightedDSU(int V, const long long *vertex_weights);
void freeWeightedDSU(WeightedDSU *dsu);
int findSet(WeightedDSU *dsu, int v);
void unionComponents(WeightedDSU *dsu, int u, int v, long long edge_w);
long long queryMaxValidComponentWeight(WeightedDSU *dsu, int K, int *best_root, int *best_size);
void processSequentialEdges(int V, int E, int K, const long long *vertex_weights, const WeightedEdge *edges);
void handleManualInput(int *V, int *E, int *K, long long **vertex_weights, WeightedEdge **edges);
void generateRandomData(int *V, int *E, int *K, long long **vertex_weights, WeightedEdge **edges);

/* Initialize WeightedDSU with individual vertex weights and component sizes of 1 */
WeightedDSU* createWeightedDSU(int V, const long long *vertex_weights) {
    WeightedDSU *dsu = (WeightedDSU *)malloc(sizeof(WeightedDSU));
    dsu->num_vertices = V;
    dsu->parent = (int *)malloc((V + 1) * sizeof(int));
    dsu->rank = (int *)calloc((V + 1), sizeof(int));
    dsu->comp_size = (int *)malloc((V + 1) * sizeof(int));
    dsu->comp_vertex_weight = (long long *)malloc((V + 1) * sizeof(long long));
    dsu->comp_edge_weight = (long long *)calloc((V + 1), sizeof(long long));

    for (int i = 1; i <= V; i++) {
        dsu->parent[i] = i;
        dsu->comp_size[i] = 1;
        dsu->comp_vertex_weight[i] = vertex_weights[i];
    }
    return dsu;
}

/* Free allocated WeightedDSU memory */
void freeWeightedDSU(WeightedDSU *dsu) {
    if (dsu) {
        free(dsu->parent);
        free(dsu->rank);
        free(dsu->comp_size);
        free(dsu->comp_vertex_weight);
        free(dsu->comp_edge_weight);
        free(dsu);
    }
}

/* Find component root with path compression */
int findSet(WeightedDSU *dsu, int v) {
    if (dsu->parent[v] != v) {
        dsu->parent[v] = findSet(dsu, dsu->parent[v]);
    }
    return dsu->parent[v];
}

/* Merge two components and aggregate their sizes, vertex weights, and edge weights */
void unionComponents(WeightedDSU *dsu, int u, int v, long long edge_w) {
    int root_u = findSet(dsu, u);
    int root_v = findSet(dsu, v);

    if (root_u == root_v) {
        dsu->comp_edge_weight[root_u] += edge_w;
        return;
    }

    if (dsu->rank[root_u] < dsu->rank[root_v]) {
        dsu->parent[root_u] = root_v;
        dsu->comp_size[root_v] += dsu->comp_size[root_u];
        dsu->comp_vertex_weight[root_v] += dsu->comp_vertex_weight[root_u];
        dsu->comp_edge_weight[root_v] += dsu->comp_edge_weight[root_u] + edge_w;
    } else if (dsu->rank[root_u] > dsu->rank[root_v]) {
        dsu->parent[root_v] = root_u;
        dsu->comp_size[root_u] += dsu->comp_size[root_v];
        dsu->comp_vertex_weight[root_u] += dsu->comp_vertex_weight[root_v];
        dsu->comp_edge_weight[root_u] += dsu->comp_edge_weight[root_v] + edge_w;
    } else {
        dsu->parent[root_v] = root_u;
        dsu->comp_size[root_u] += dsu->comp_size[root_v];
        dsu->comp_vertex_weight[root_u] += dsu->comp_vertex_weight[root_v];
        dsu->comp_edge_weight[root_u] += dsu->comp_edge_weight[root_v] + edge_w;
        dsu->rank[root_u]++;
    }
}

/*
 * Compute the maximum total vertex weight among all current connected components
 * whose vertex count (size) does not exceed the threshold K.
 * Returns LLONG_MIN if no component has size <= K.
 */
long long queryMaxValidComponentWeight(WeightedDSU *dsu, int K, int *best_root, int *best_size) {
    long long max_weight = LLONG_MIN;
    *best_root = -1;
    *best_size = 0;

    for (int i = 1; i <= dsu->num_vertices; i++) {
        if (dsu->parent[i] == i && dsu->comp_size[i] <= K) {
            if (dsu->comp_vertex_weight[i] > max_weight) {
                max_weight = dsu->comp_vertex_weight[i];
                *best_root = i;
                *best_size = dsu->comp_size[i];
            }
        }
    }
    return max_weight;
}

/* Process sequential edge additions and report maximum valid component weight at each point */
void processSequentialEdges(int V, int E, int K, const long long *vertex_weights, const WeightedEdge *edges) {
    WeightedDSU *dsu = createWeightedDSU(V, vertex_weights);
    int best_root = -1, best_size = 0;

    printf("\n--- Maximum Weight Component (Size <= %d) Log ---\n", K);
    long long init_max = queryMaxValidComponentWeight(dsu, K, &best_root, &best_size);
    if (init_max != LLONG_MIN) {
        printf("Initial State (0 edges) -> Max Total Vertex Weight = %lld (Root = %d, Size = %d)\n",
               init_max, best_root, best_size);
    } else {
        printf("Initial State (0 edges) -> No component has size <= %d\n", K);
    }

    for (int i = 0; i < E; i++) {
        int u = edges[i].u;
        int v = edges[i].v;
        long long w = edges[i].edge_weight;

        if (u < 1 || u > V || v < 1 || v > V) {
            printf("Step %2d: Edge (%d, %d, w=%lld) -> INVALID VERTEX ID (Skipped)\n", i + 1, u, v, w);
            continue;
        }

        unionComponents(dsu, u, v, w);
        long long current_max = queryMaxValidComponentWeight(dsu, K, &best_root, &best_size);

        if (current_max != LLONG_MIN) {
            printf("Step %2d: Added Edge (%d, %d, w=%lld) -> Max Valid Component Vertex Weight = %lld (Root = %d, Size = %d)\n",
                   i + 1, u, v, w, current_max, best_root, best_size);
        } else {
            printf("Step %2d: Added Edge (%d, %d, w=%lld) -> No component has size <= %d\n",
                   i + 1, u, v, w, K);
        }
    }

    freeWeightedDSU(dsu);
}

/* Handle manual user input */
void handleManualInput(int *V, int *E, int *K, long long **vertex_weights, WeightedEdge **edges) {
    printf("Enter number of vertices (V >= 1), number of edges (E >= 1), and size threshold (K >= 1): ");
    if (scanf("%d %d %d", V, E, K) != 3 || *V < 1 || *E < 1 || *K < 1) {
        fprintf(stderr, "Invalid input parameters.\n");
        exit(EXIT_FAILURE);
    }

    *vertex_weights = (long long *)malloc((*V + 1) * sizeof(long long));
    printf("Enter the vertex weights w_1, w_2, ..., w_%d:\n", *V);
    for (int i = 1; i <= *V; i++) {
        if (scanf("%lld", &((*vertex_weights)[i])) != 1) {
            fprintf(stderr, "Invalid vertex weight input.\n");
            free(*vertex_weights);
            exit(EXIT_FAILURE);
        }
    }

    *edges = (WeightedEdge *)malloc((*E) * sizeof(WeightedEdge));
    printf("Enter %d sequential edges as 'u v weight' (1-indexed vertices 1 to %d):\n", *E, *V);
    for (int i = 0; i < *E; i++) {
        printf("Edge %d: ", i + 1);
        if (scanf("%d %d %lld", &((*edges)[i].u), &((*edges)[i].v), &((*edges)[i].edge_weight)) != 3) {
            fprintf(stderr, "Invalid edge input.\n");
            free(*vertex_weights);
            free(*edges);
            exit(EXIT_FAILURE);
        }
    }
}

/* Generate randomized graph data adhering to problem constraints */
void generateRandomData(int *V, int *E, int *K, long long **vertex_weights, WeightedEdge **edges) {
    srand((unsigned int)time(NULL));
    *V = 6 + (rand() % 5);          /* 6 to 10 vertices */
    *E = *V + (rand() % 4);         /* V to V+3 edges */
    *K = 2 + (rand() % (*V / 2));   /* Size threshold K between 2 and V/2 + 1 */

    *vertex_weights = (long long *)malloc((*V + 1) * sizeof(long long));
    printf("\nGenerated Parameters: V = %d, E = %d, Threshold K = %d\n", *V, *E, *K);
    printf("Generated Vertex Weights: ");
    for (int i = 1; i <= *V; i++) {
        (*vertex_weights)[i] = 10 + (rand() % 90); /* Weights between 10 and 99 */
        printf("w[%d]=%lld%s", i, (*vertex_weights)[i], (i == *V) ? "\n" : ", ");
    }

    *edges = (WeightedEdge *)malloc((*E) * sizeof(WeightedEdge));
    printf("Generated Sequential Edges:\n");
    for (int i = 0; i < *E; i++) {
        int u = 1 + (rand() % (*V));
        int v = 1 + (rand() % (*V));
        while (v == u && *V > 1) {
            v = 1 + (rand() % (*V));
        }
        long long w = 1 + (rand() % 50);
        (*edges)[i].u = u;
        (*edges)[i].v = v;
        (*edges)[i].edge_weight = w;
        printf("  Edge %2d: (%d, %d, weight = %lld)\n", i + 1, u, v, w);
    }
}

int main(void) {
    int choice;
    int V = 0, E = 0, K = 0;
    long long *vertex_weights = NULL;
    WeightedEdge *edges = NULL;

    printf("=============================================================\n");
    printf(" Q03: Maximum Weight Component with Size Constraints (<= K)\n");
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
        handleManualInput(&V, &E, &K, &vertex_weights, &edges);
    } else if (choice == 2) {
        generateRandomData(&V, &E, &K, &vertex_weights, &edges);
    } else {
        fprintf(stderr, "Invalid choice. Please select 1 or 2.\n");
        return EXIT_FAILURE;
    }

    processSequentialEdges(V, E, K, vertex_weights, edges);

    free(vertex_weights);
    free(edges);
    return EXIT_SUCCESS;
}
