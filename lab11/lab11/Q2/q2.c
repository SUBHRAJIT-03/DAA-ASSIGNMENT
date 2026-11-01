/*
 * DAA Lab 11 - Q02: Multi-Source BFS for Radius-Constrained Voronoi Cell Partitioning
 *
 * Description:
 * Partitions an unweighted, undirected graph G = (V, E) into radius-constrained
 * discrete Voronoi cells around K designated hub vertices H = {h_1, ..., h_K}
 * within radius threshold R >= 0, breaking distance ties by smallest hub index.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>

#define INF INT_MAX

/* Adjacency list node */
typedef struct Node {
    int vertex;
    struct Node *next;
} Node;

/* Problem instance structure */
typedef struct {
    int num_vertices;
    int num_edges;
    int num_hubs;
    int radius;
    int *hubs;      /* 1-indexed array of size num_hubs + 1: hubs[i] is vertex ID */
    Node **adj;     /* 1-indexed adjacency list */
} VoronoiGraph;

/* --- Graph & Memory Helper Functions --- */

VoronoiGraph *create_voronoi_graph(int v, int e, int k, int r) {
    VoronoiGraph *g = (VoronoiGraph *)malloc(sizeof(VoronoiGraph));
    if (!g) {
        perror("Failed to allocate VoronoiGraph");
        exit(EXIT_FAILURE);
    }
    g->num_vertices = v;
    g->num_edges = e;
    g->num_hubs = k;
    g->radius = r;
    g->hubs = (int *)calloc(k + 1, sizeof(int));
    g->adj = (Node **)calloc(v + 1, sizeof(Node *));
    if (!g->hubs || !g->adj) {
        perror("Failed to allocate graph arrays");
        exit(EXIT_FAILURE);
    }
    return g;
}

void add_undirected_edge(VoronoiGraph *g, int u, int v) {
    Node *n1 = (Node *)malloc(sizeof(Node));
    Node *n2 = (Node *)malloc(sizeof(Node));
    if (!n1 || !n2) {
        perror("Failed to allocate adjacency node");
        exit(EXIT_FAILURE);
    }
    n1->vertex = v;
    n1->next = g->adj[u];
    g->adj[u] = n1;

    n2->vertex = u;
    n2->next = g->adj[v];
    g->adj[v] = n2;
}

void free_voronoi_graph(VoronoiGraph *g) {
    if (!g) return;
    for (int i = 1; i <= g->num_vertices; i++) {
        Node *curr = g->adj[i];
        while (curr) {
            Node *tmp = curr;
            curr = curr->next;
            free(tmp);
        }
    }
    free(g->adj);
    free(g->hubs);
    free(g);
}

void print_instance_summary(const VoronoiGraph *g) {
    printf("\n--- Graph & Hubs Summary ---\n");
    printf("Vertices (|V|): %d | Undirected Edges (|E|): %d | Hubs (K): %d | Radius (R): %d\n",
           g->num_vertices, g->num_edges, g->num_hubs, g->radius);
    printf("Designated Hubs H = { ");
    for (int i = 1; i <= g->num_hubs; i++) {
        printf("h_%d=%d%s", i, g->hubs[i], (i < g->num_hubs) ? ", " : " ");
    }
    printf("}\n");

    if (g->num_vertices <= 25) {
        printf("Adjacency List:\n");
        for (int u = 1; u <= g->num_vertices; u++) {
            printf("  Vertex %2d: ", u);
            Node *curr = g->adj[u];
            if (!curr) printf("(isolated)");
            while (curr) {
                printf("%d ", curr->vertex);
                curr = curr->next;
            }
            printf("\n");
        }
    } else {
        printf("(Adjacency list printing omitted for |V| > 25)\n");
    }
}

/* --- Input Handling Functions --- */

VoronoiGraph *handle_manual_input(void) {
    int v, e, k, r;

    printf("\n[Manual Input Mode]\n");
    printf("Enter number of vertices |V| (>= 1): ");
    if (scanf("%d", &v) != 1 || v < 1) {
        fprintf(stderr, "Invalid number of vertices.\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter number of undirected edges |E| (>= 0): ");
    if (scanf("%d", &e) != 1 || e < 0) {
        fprintf(stderr, "Invalid number of edges.\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter number of designated hubs K (1 to %d): ", v);
    if (scanf("%d", &k) != 1 || k < 1 || k > v) {
        fprintf(stderr, "Invalid number of hubs K. Must be between 1 and %d.\n", v);
        exit(EXIT_FAILURE);
    }

    printf("Enter integer radius threshold R (>= 0): ");
    if (scanf("%d", &r) != 1 || r < 0) {
        fprintf(stderr, "Invalid radius threshold R. Must be >= 0.\n");
        exit(EXIT_FAILURE);
    }

    VoronoiGraph *g = create_voronoi_graph(v, e, k, r);

    printf("Enter %d hub vertices (h_1 to h_%d, each between 1 and %d):\n", k, k, v);
    for (int i = 1; i <= k; i++) {
        if (scanf("%d", &g->hubs[i]) != 1 || g->hubs[i] < 1 || g->hubs[i] > v) {
            fprintf(stderr, "Invalid hub vertex at index %d.\n", i);
            free_voronoi_graph(g);
            exit(EXIT_FAILURE);
        }
    }

    printf("Enter %d undirected edges in format: u v (where 1 <= u, v <= %d):\n", e, v);
    for (int i = 0; i < e; i++) {
        int u, to;
        if (scanf("%d %d", &u, &to) != 2 || u < 1 || u > v || to < 1 || to > v) {
            fprintf(stderr, "Invalid edge at index %d.\n", i + 1);
            free_voronoi_graph(g);
            exit(EXIT_FAILURE);
        }
        add_undirected_edge(g, u, to);
    }

    return g;
}

VoronoiGraph *generate_random_data(void) {
    int v, e, k, r;

    printf("\n[Randomized Data Generation Mode]\n");
    printf("Enter number of vertices |V| (e.g., 8 to 1000): ");
    if (scanf("%d", &v) != 1 || v < 1) {
        fprintf(stderr, "Invalid number of vertices.\n");
        exit(EXIT_FAILURE);
    }

    int max_edges = (v <= 1000) ? (v * (v - 1)) / 2 : 1000000;
    printf("Enter number of undirected edges |E| (0 to %d): ", max_edges);
    if (scanf("%d", &e) != 1 || e < 0) {
        fprintf(stderr, "Invalid number of edges.\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter number of designated hubs K (1 to %d): ", v);
    if (scanf("%d", &k) != 1 || k < 1 || k > v) {
        fprintf(stderr, "Invalid number of hubs K.\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter radius threshold R (>= 0): ");
    if (scanf("%d", &r) != 1 || r < 0) {
        fprintf(stderr, "Invalid radius threshold R.\n");
        exit(EXIT_FAILURE);
    }

    srand((unsigned int)time(NULL));
    VoronoiGraph *g = create_voronoi_graph(v, e, k, r);

    /* Select K distinct hub vertices using partial Fisher-Yates shuffle */
    int *perm = (int *)malloc(v * sizeof(int));
    if (!perm) {
        perror("Failed to allocate permutation array");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < v; i++) perm[i] = i + 1;
    for (int i = 0; i < k; i++) {
        int j = i + (rand() % (v - i));
        int tmp = perm[i];
        perm[i] = perm[j];
        perm[j] = tmp;
        g->hubs[i + 1] = perm[i];
    }
    free(perm);

    /* Generate random undirected edges */
    for (int i = 0; i < e; i++) {
        int u = (rand() % v) + 1;
        int to = (rand() % v) + 1;
        if (v > 1 && u == to) {
            to = (u % v) + 1;
        }
        add_undirected_edge(g, u, to);
    }

    return g;
}

/* --- Core Algorithmic Logic: Multi-Source BFS with Tie-Breaking --- */

void solve_multi_source_voronoi_bfs(const VoronoiGraph *g) {
    int n = g->num_vertices;
    int *dist = (int *)malloc((n + 1) * sizeof(int));
    int *owner = (int *)malloc((n + 1) * sizeof(int));
    int *queue = (int *)malloc((n + 1) * sizeof(int));

    if (!dist || !owner || !queue) {
        perror("Failed to allocate BFS arrays");
        exit(EXIT_FAILURE);
    }

    for (int v = 1; v <= n; v++) {
        dist[v] = INF;
        owner[v] = -1;
    }

    int front = 0, back = 0;

    /* Enqueue hubs in increasing order of hub index i = 1..K.
     * This establishes the level-0 tie-breaking invariant: hubs with smaller
     * indices are processed first at every distance layer. */
    for (int i = 1; i <= g->num_hubs; i++) {
        int h = g->hubs[i];
        if (dist[h] == INF) {
            dist[h] = 0;
            owner[h] = i;
            queue[back++] = h;
        }
    }

    /* Multi-Source BFS bounded by radius threshold R */
    while (front < back) {
        int u = queue[front++];

        /* Prune expansion if current vertex has already reached radius R */
        if (dist[u] >= g->radius) {
            continue;
        }

        for (Node *curr = g->adj[u]; curr != NULL; curr = curr->next) {
            int v = curr->vertex;
            if (dist[v] == INF) {
                dist[v] = dist[u] + 1;
                owner[v] = owner[u];
                queue[back++] = v;
            } else if (dist[v] == dist[u] + 1 && owner[u] < owner[v]) {
                /* Explicit tie-breaking safeguard for equal shortest distances */
                owner[v] = owner[u];
            }
        }
    }

    /* Output Results */
    printf("\n--- Voronoi Cell Partitioning Results (Radius R = %d) ---\n", g->radius);
    printf("%-12s | %-18s | %-16s | %-20s\n",
           "Vertex (v)", "owner[v] (Hub ID)", "Hub Vertex (h_i)", "Shortest Dist d_min");
    printf("---------------------------------------------------------------------------\n");

    for (int v = 1; v <= n; v++) {
        if (owner[v] == -1) {
            printf("%-12d | %-18d | %-16s | %-20s\n",
                   v, -1, "N/A", "OUTSIDE (> R / INF)");
        } else {
            printf("%-12d | %-18d | %-16d | %-20d\n",
                   v, owner[v], g->hubs[owner[v]], dist[v]);
        }
    }

    free(dist);
    free(owner);
    free(queue);
}

/* --- Menu-Driven Entry Point --- */

int main(void) {
    int choice;
    printf("=========================================================================\n");
    printf("  Q02: Multi-Source BFS for Radius-Constrained Voronoi Cell Partitioning\n");
    printf("=========================================================================\n");
    printf("Select Input Method:\n");
    printf("  1. Manual user input\n");
    printf("  2. Randomized data generation\n");
    printf("Enter choice (1 or 2): ");

    if (scanf("%d", &choice) != 1 || (choice != 1 && choice != 2)) {
        fprintf(stderr, "Invalid choice. Please enter 1 or 2.\n");
        return EXIT_FAILURE;
    }

    VoronoiGraph *g = NULL;
    if (choice == 1) {
        g = handle_manual_input();
    } else {
        g = generate_random_data();
    }

    print_instance_summary(g);
    solve_multi_source_voronoi_bfs(g);
    free_voronoi_graph(g);

    return EXIT_SUCCESS;
}
