#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>

#define INF LLONG_MAX

/* Adjacency list edge structure */
typedef struct Edge {
    int to;
    int weight; /* 0 or 1 */
    struct Edge *next;
} Edge;

/* Directed 0-1 graph structure */
typedef struct {
    int num_vertices;
    int num_edges;
    int source;
    long long penalty_c;
    Edge **head;
} Graph;

/* State for 0-1 BFS: vertex u and parity p in {0, 1} of weight-1 edges */
typedef struct {
    int vertex;
    int parity;
} State;

/* Double-ended queue (Deque) for 0-1 BFS */
typedef struct {
    State *data;
    int front;
    int back;
    int capacity;
} Deque;

/* --- Graph & Memory Helper Functions --- */

Graph *create_graph(int v, int e, int source, long long penalty_c) {
    Graph *g = (Graph *)malloc(sizeof(Graph));
    if (!g) {
        perror("Failed to allocate Graph");
        exit(EXIT_FAILURE);
    }
    g->num_vertices = v;
    g->num_edges = e;
    g->source = source;
    g->penalty_c = penalty_c;
    g->head = (Edge **)calloc(v + 1, sizeof(Edge *));
    if (!g->head) {
        perror("Failed to allocate adjacency list");
        free(g);
        exit(EXIT_FAILURE);
    }
    return g;
}

void add_directed_edge(Graph *g, int u, int v, int w) {
    Edge *new_edge = (Edge *)malloc(sizeof(Edge));
    if (!new_edge) {
        perror("Failed to allocate Edge");
        exit(EXIT_FAILURE);
    }
    new_edge->to = v;
    new_edge->weight = w;
    new_edge->next = g->head[u];
    g->head[u] = new_edge;
}

void free_graph(Graph *g) {
    if (!g) return;
    for (int i = 1; i <= g->num_vertices; i++) {
        Edge *curr = g->head[i];
        while (curr) {
            Edge *temp = curr;
            curr = curr->next;
            free(temp);
        }
    }
    free(g->head);
    free(g);
}

void print_graph_summary(const Graph *g) {
    printf("\nGraph Summary: \n");
    printf("Vertices (|V|): %d | Directed Edges (|E|): %d | Source (S): %d | Odd Penalty (C): %lld\n",
           g->num_vertices, g->num_edges, g->source, g->penalty_c);
    if (g->num_vertices <= 25) {
        printf("Adjacency List (u -> (v, weight)):\n");
        for (int u = 1; u <= g->num_vertices; u++) {
            printf("  Vertex %2d: ", u);
            Edge *curr = g->head[u];
            if (!curr) {
                printf("(no outgoing edges)");
            }
            while (curr) {
                printf("-> (%d, w=%d) ", curr->to, curr->weight);
                curr = curr->next;
            }
            printf("\n");
        }
    } else {
        printf("(Adjacency list printing omitted for |V| > 25)\n");
    }
}

/* --- Deque Helper Functions for 0-1 BFS --- */

Deque *create_deque(int max_elements) {
    Deque *dq = (Deque *)malloc(sizeof(Deque));
    if (!dq) {
        perror("Failed to allocate Deque");
        exit(EXIT_FAILURE);
    }
    /* Allocate 2 * max_elements + 5 so front and back can grow freely without wrap-around */
    dq->capacity = 2 * max_elements + 5;
    dq->data = (State *)malloc(dq->capacity * sizeof(State));
    if (!dq->data) {
        perror("Failed to allocate Deque buffer");
        free(dq);
        exit(EXIT_FAILURE);
    }
    dq->front = max_elements + 2;
    dq->back = max_elements + 2;
    return dq;
}

bool is_deque_empty(const Deque *dq) {
    return dq->front == dq->back;
}

void push_front(Deque *dq, int vertex, int parity) {
    dq->front--;
    dq->data[dq->front].vertex = vertex;
    dq->data[dq->front].parity = parity;
}

void push_back(Deque *dq, int vertex, int parity) {
    dq->data[dq->back].vertex = vertex;
    dq->data[dq->back].parity = parity;
    dq->back++;
}

State pop_front(Deque *dq) {
    State s = dq->data[dq->front];
    dq->front++;
    return s;
}

void free_deque(Deque *dq) {
    if (!dq) return;
    free(dq->data);
    free(dq);
}

/* --- Input Handling Functions --- */

Graph *handle_manual_input(void) {
    int v, e, s;
    long long c;

    printf("\n[Manual Input Mode]\n");
    printf("Enter number of vertices |V| (>= 1): ");
    if (scanf("%d", &v) != 1 || v < 1) {
        fprintf(stderr, "Invalid number of vertices.\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter number of directed edges |E| (>= 0): ");
    if (scanf("%d", &e) != 1 || e < 0) {
        fprintf(stderr, "Invalid number of edges.\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter designated source vertex S (1 to %d): ", v);
    if (scanf("%d", &s) != 1 || s < 1 || s > v) {
        fprintf(stderr, "Invalid source vertex. Must be between 1 and %d.\n", v);
        exit(EXIT_FAILURE);
    }

    printf("Enter extra penalty cost C (>= 0): ");
    if (scanf("%lld", &c) != 1 || c < 0) {
        fprintf(stderr, "Invalid penalty cost. Must be >= 0.\n");
        exit(EXIT_FAILURE);
    }

    Graph *g = create_graph(v, e, s, c);

    printf("Enter %d directed edges in format: u v w (where 1 <= u,v <= %d and w in {0, 1}):\n", e, v);
    for (int i = 0; i < e; i++) {
        int u, to, w;
        if (scanf("%d %d %d", &u, &to, &w) != 3 ||
            u < 1 || u > v || to < 1 || to > v || (w != 0 && w != 1)) {
            fprintf(stderr, "Invalid edge at index %d. Expected u, v in [1, %d] and w in {0, 1}.\n", i + 1, v);
            free_graph(g);
            exit(EXIT_FAILURE);
        }
        add_directed_edge(g, u, to, w);
    }

    return g;
}

Graph *generate_random_data(void) {
    int v, e, s;
    long long c;

    printf("\n[Randomized Data Generation Mode]\n");
    printf("Enter number of vertices |V| (e.g., 6 to 1000): ");
    if (scanf("%d", &v) != 1 || v < 1) {
        fprintf(stderr, "Invalid number of vertices.\n");
        exit(EXIT_FAILURE);
    }

    int max_edges = (v <= 1000) ? v * (v - 1) : 1000000;
    printf("Enter number of directed edges |E| (0 to %d): ", max_edges);
    if (scanf("%d", &e) != 1 || e < 0) {
        fprintf(stderr, "Invalid number of edges.\n");
        exit(EXIT_FAILURE);
    }

    srand((unsigned int)time(NULL));
    s = (rand() % v) + 1;
    c = rand() % 15; /* Non-negative penalty C >= 0 */

    Graph *g = create_graph(v, e, s, c);

    for (int i = 0; i < e; i++) {
        int u = (rand() % v) + 1;
        int to = (rand() % v) + 1;
        if (v > 1 && u == to) {
            to = (u % v) + 1;
        }
        int w = rand() % 2; /* Strictly in {0, 1} */
        add_directed_edge(g, u, to, w);
    }

    return g;
}

/* --- Core Algorithmic Logic: Parity-Aware 0-1 BFS --- */

void solve_parity_constrained_01bfs(const Graph *g) {
    int n = g->num_vertices;

    /* dist[u][0] = min W(P) to u with even k1(P)
     * dist[u][1] = min W(P) to u with odd  k1(P) */
    long long (*dist)[2] = malloc((n + 1) * sizeof(*dist));
    bool (*visited)[2] = calloc(n + 1, sizeof(*visited));
    if (!dist || !visited) {
        perror("Failed to allocate 0-1 BFS tables");
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i <= n; i++) {
        dist[i][0] = INF;
        dist[i][1] = INF;
    }

    /* Each state (u, parity) can be pushed at most |E| + 1 times;
     * since edge weights are 0 or 1, total pushes <= 2*|V| + 2*|E|. */
    int max_pushes = 2 * (n + g->num_edges) + 10;
    Deque *dq = create_deque(max_pushes);

    dist[g->source][0] = 0;
    push_front(dq, g->source, 0);

    while (!is_deque_empty(dq)) {
        State curr = pop_front(dq);
        int u = curr.vertex;
        int p = curr.parity;

        if (visited[u][p]) {
            continue;
        }
        visited[u][p] = true;

        long long d = dist[u][p];

        for (Edge *e = g->head[u]; e != NULL; e = e->next) {
            int v = e->to;
            int w = e->weight;
            int next_parity = p ^ w; /* Flips parity iff w == 1 */
            long long next_dist = d + w;

            if (next_dist < dist[v][next_parity]) {
                dist[v][next_parity] = next_dist;
                if (w == 0) {
                    push_front(dq, v, next_parity);
                } else {
                    push_back(dq, v, next_parity);
                }
            }
        }
    }

    /* Output Results */
    printf("\n--- Minimum Effective Cost from Source S = %d (Penalty C = %lld) ---\n",
           g->source, g->penalty_c);
    printf("%-10s | %-18s | %-18s | %-20s\n",
           "Vertex (v)", "Even k1 W(P)", "Odd k1 W(P)+C", "Min Effective Cost");
    printf("--------------------------------------------------------------------------\n");

    for (int v = 1; v <= n; v++) {
        long long even_cost = dist[v][0];
        long long odd_cost = (dist[v][1] == INF) ? INF : (dist[v][1] + g->penalty_c);
        long long best_cost = (even_cost < odd_cost) ? even_cost : odd_cost;

        char even_str[80], odd_str[80], best_str[80];
        if (even_cost == INF) snprintf(even_str, sizeof(even_str), "INF");
        else snprintf(even_str, sizeof(even_str), "%lld", even_cost);

        if (odd_cost == INF) snprintf(odd_str, sizeof(odd_str), "INF");
        else snprintf(odd_str, sizeof(odd_str), "%lld (%lld+%lld)", odd_cost, dist[v][1], g->penalty_c);

        if (best_cost == INF) snprintf(best_str, sizeof(best_str), "UNREACHABLE");
        else snprintf(best_str, sizeof(best_str), "%lld", best_cost);

        printf("%-10d | %-18s | %-18s | %-20s\n", v, even_str, odd_str, best_str);
    }

    free_deque(dq);
    free(visited);
    free(dist);
}

/* --- Menu-Driven Entry Point --- */

int main(void) {
    int choice;
    printf("=================================================================\n");
    printf("  Q01: Shortest Path on Parity-Constrained Graphs via 0-1 BFS\n");
    printf("=================================================================\n");
    printf("Select Input Method:\n");
    printf("  1. Manual user input\n");
    printf("  2. Randomized data generation\n");
    printf("Enter choice (1 or 2): ");

    if (scanf("%d", &choice) != 1 || (choice != 1 && choice != 2)) {
        fprintf(stderr, "Invalid choice. Please enter 1 or 2.\n");
        return EXIT_FAILURE;
    }

    Graph *g = NULL;
    if (choice == 1) {
        g = handle_manual_input();
    } else {
        g = generate_random_data();
    }

    print_graph_summary(g);
    solve_parity_constrained_01bfs(g);
    free_graph(g);

    return EXIT_SUCCESS;
}
