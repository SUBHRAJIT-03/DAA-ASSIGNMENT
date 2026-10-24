#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* Structure representing an undirected edge arriving online */
typedef struct {
    int u;
    int v;
} Edge;

/* Structure representing a Parity-Augmented Disjoint Set Union (DSU) */
typedef struct {
    int *parent;
    int *rank;
    int *parity; /* XOR parity of path from node to its parent */
    int num_vertices;
} BipartiteDSU;

/* Result pair returned by findSet: root of component and parity to root */
typedef struct {
    int root;
    int parity_to_root;
} FindResult;

/* Function Prototypes */
BipartiteDSU* createBipartiteDSU(int V);
void freeBipartiteDSU(BipartiteDSU *dsu);
FindResult findSet(BipartiteDSU *dsu, int v);
bool addEdgeOnline(BipartiteDSU *dsu, int u, int v);
void processOnlineEdges(int V, int E, Edge *edges);
void handleManualInput(int *V, int *E, Edge **edges);
void generateRandomData(int *V, int *E, Edge **edges);

/* Initialize Parity-Augmented DSU for V vertices (1-indexed: 1 to V) */
BipartiteDSU* createBipartiteDSU(int V) {
    BipartiteDSU *dsu = (BipartiteDSU *)malloc(sizeof(BipartiteDSU));
    dsu->num_vertices = V;
    dsu->parent = (int *)malloc((V + 1) * sizeof(int));
    dsu->rank = (int *)calloc((V + 1), sizeof(int));
    dsu->parity = (int *)calloc((V + 1), sizeof(int));

    for (int i = 1; i <= V; i++) {
        dsu->parent[i] = i;
    }
    return dsu;
}

/* Free allocated DSU memory */
void freeBipartiteDSU(BipartiteDSU *dsu) {
    if (dsu) {
        free(dsu->parent);
        free(dsu->rank);
        free(dsu->parity);
        free(dsu);
    }
}

/* Find root of vertex v with path compression and parity accumulation */
FindResult findSet(BipartiteDSU *dsu, int v) {
    if (v == dsu->parent[v]) {
        FindResult res = {v, 0};
        return res;
    }
    FindResult parent_res = findSet(dsu, dsu->parent[v]);
    dsu->parent[v] = parent_res.root;
    dsu->parity[v] ^= parent_res.parity_to_root;

    FindResult res = {dsu->parent[v], dsu->parity[v]};
    return res;
}

/*
 * Attempt to add an edge (u, v) online.
 * Returns true if the edge maintains bipartiteness (and adds it),
 * or false if adding (u, v) introduces an odd-length cycle (skips it).
 */
bool addEdgeOnline(BipartiteDSU *dsu, int u, int v) {
    if (u == v) {
        /* A self-loop is a cycle of length 1 (odd) */
        return false;
    }

    FindResult res_u = findSet(dsu, u);
    FindResult res_v = findSet(dsu, v);

    if (res_u.root == res_v.root) {
        /* If u and v have the same parity to the root, adding (u, v) forms an odd cycle */
        if (res_u.parity_to_root == res_v.parity_to_root) {
            return false;
        }
        /* Already in the same component with opposite colors; edge is valid */
        return true;
    }

    /* Union by rank while maintaining relative bipartite parity */
    int root_u = res_u.root;
    int root_v = res_v.root;

    if (dsu->rank[root_u] < dsu->rank[root_v]) {
        dsu->parent[root_u] = root_v;
        dsu->parity[root_u] = res_u.parity_to_root ^ res_v.parity_to_root ^ 1;
    } else if (dsu->rank[root_u] > dsu->rank[root_v]) {
        dsu->parent[root_v] = root_u;
        dsu->parity[root_v] = res_u.parity_to_root ^ res_v.parity_to_root ^ 1;
    } else {
        dsu->parent[root_v] = root_u;
        dsu->parity[root_v] = res_u.parity_to_root ^ res_v.parity_to_root ^ 1;
        dsu->rank[root_u]++;
    }
    return true;
}

/* Process a sequence of online edge arrivals and print validation status */
void processOnlineEdges(int V, int E, Edge *edges) {
    BipartiteDSU *dsu = createBipartiteDSU(V);
    int accepted_count = 0;
    int skipped_count = 0;

    printf("\n--- Online Graph-Bipartiteness Testing Log ---\n");
    for (int i = 0; i < E; i++) {
        int u = edges[i].u;
        int v = edges[i].v;

        if (u < 1 || u > V || v < 1 || v > V) {
            printf("Step %2d: Edge (%d, %d) -> INVALID VERTEX ID (Skipped)\n", i + 1, u, v);
            skipped_count++;
            continue;
        }

        if (addEdgeOnline(dsu, u, v)) {
            accepted_count++;
            printf("Step %2d: Edge (%d, %d) -> ADDED   | Graph remains bipartite.\n", i + 1, u, v);
        } else {
            skipped_count++;
            printf("Step %2d: Edge (%d, %d) -> SKIPPED | Violates bipartiteness (introduces an odd cycle).\n", i + 1, u, v);
        }
    }

    printf("\nSummary: %d edge(s) added, %d edge(s) skipped. Final graph is bipartite.\n",
           accepted_count, skipped_count);

    freeBipartiteDSU(dsu);
}

/* Handle manual user input */
void handleManualInput(int *V, int *E, Edge **edges) {
    printf("Enter the number of vertices (V >= 1): ");
    if (scanf("%d", V) != 1 || *V < 1) {
        fprintf(stderr, "Invalid number of vertices.\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter the number of online edges arriving (E >= 1): ");
    if (scanf("%d", E) != 1 || *E < 1) {
        fprintf(stderr, "Invalid number of edges.\n");
        exit(EXIT_FAILURE);
    }

    *edges = (Edge *)malloc((*E) * sizeof(Edge));
    printf("Enter %d edges as pairs 'u v' (1-indexed vertices from 1 to %d):\n", *E, *V);
    for (int i = 0; i < *E; i++) {
        printf("Edge %d: ", i + 1);
        if (scanf("%d %d", &((*edges)[i].u), &((*edges)[i].v)) != 2) {
            fprintf(stderr, "Invalid edge input.\n");
            free(*edges);
            exit(EXIT_FAILURE);
        }
    }
}

/* Generate randomized graph data adhering to problem constraints */
void generateRandomData(int *V, int *E, Edge **edges) {
    srand((unsigned int)time(NULL));
    *V = 6 + (rand() % 5);        /* 6 to 10 vertices */
    *E = *V + 2 + (rand() % 6);   /* V+2 to V+7 online edges */

    *edges = (Edge *)malloc((*E) * sizeof(Edge));
    printf("\nGenerated Random Parameters: V = %d vertices, E = %d online edges.\n", *V, *E);
    printf("Generated Edge Stream: ");
    for (int i = 0; i < *E; i++) {
        int u = 1 + (rand() % (*V));
        int v = 1 + (rand() % (*V));
        while (v == u && *V > 1) {
            v = 1 + (rand() % (*V));
        }
        (*edges)[i].u = u;
        (*edges)[i].v = v;
        printf("(%d, %d)%s", u, v, (i == *E - 1) ? "\n" : ", ");
    }
}

int main(void) {
    int choice;
    int V = 0, E = 0;
    Edge *edges = NULL;

    printf("=============================================================\n");
    printf(" Q01: Online Graph-Bipartiteness Testing (Parity-Based DSU)\n");
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
        handleManualInput(&V, &E, &edges);
    } else if (choice == 2) {
        generateRandomData(&V, &E, &edges);
    } else {
        fprintf(stderr, "Invalid choice. Please select 1 or 2.\n");
        return EXIT_FAILURE;
    }

    processOnlineEdges(V, E, edges);

    free(edges);
    return EXIT_SUCCESS;
}
