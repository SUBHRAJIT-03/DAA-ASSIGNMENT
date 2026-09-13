#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int allowed_toggle(int mask, int n, int i) {
    if (i == n - 1) return 1;
    if (!((mask >> (n - 1 - (i + 1))) & 1)) return 0;
    for (int j = i + 2; j < n; ++j) {
        if ((mask >> (n - 1 - j)) & 1) return 0;
    }
    return 1;
}

static int read_input_mode(void) {
    int mode = 0;
    printf("Security switches — find minimal toggles to turn all off\n");
    printf("Choose input mode:\n 1) Manual n\n 2) Random n (small)\nEnter 1 or 2: ");
    if (scanf("%d", &mode) != 1) return -1;
    return mode;
}

static int read_manual_n(void) {
    int n = 0;
    printf("Enter number of switches n (recommended <=20): ");
    if (scanf("%d", &n) != 1) return -1;
    return n;
}

static int generate_random_n(void) {
    int maxn = 15;
    printf("Enter max n for random generation (default 15): ");
    if (scanf("%d", &maxn) != 1) maxn = 15;
    if (maxn < 1) maxn = 15;
    int n = (rand() % maxn) + 1;
    printf("Random n = %d\n", n);
    return n;
}

static int solve_security_switches(int n) {
    if (n < 1 || n > 24) {
        printf("n must be between 1 and 24 (practical limit).\n");
        return 1;
    }

    int N = 1 << n;
    int start = N - 1;
    int *dist = malloc(sizeof(int) * N);
    int *parent = malloc(sizeof(int) * N);
    int *parent_move = malloc(sizeof(int) * N);
    if (!dist || !parent || !parent_move) {
        perror("malloc");
        free(dist); free(parent); free(parent_move);
        return 1;
    }

    for (int i = 0; i < N; ++i) {
        dist[i] = -1; parent[i] = -1; parent_move[i] = -1;
    }

    int *queue = malloc(sizeof(int) * N);
    if (!queue) {
        perror("malloc");
        free(dist); free(parent); free(parent_move);
        return 1;
    }

    int qh = 0, qt = 0;
    queue[qt++] = start; dist[start] = 0;

    while (qh < qt) {
        int mask = queue[qh++];
        if (mask == 0) break;
        for (int i = 0; i < n; ++i) {
            if (!allowed_toggle(mask, n, i)) continue;
            int bit = n - 1 - i;
            int nmask = mask ^ (1 << bit);
            if (dist[nmask] == -1) {
                dist[nmask] = dist[mask] + 1;
                parent[nmask] = mask;
                parent_move[nmask] = i + 1;
                queue[qt++] = nmask;
            }
        }
    }

    if (dist[0] == -1) {
        printf("No sequence found (this should not happen for tested n).\n");
    } else {
        printf("Minimal moves = %d\n", dist[0]);
        int path_len = dist[0];
        int *moves = malloc(sizeof(int) * path_len);
        int cur = 0; int p = 0;
        while (cur != start) {
            moves[p++] = parent_move[cur];
            cur = parent[cur];
        }
        printf("Sequence of toggles (left-indexed 1..n):\n");
        for (int i = p - 1; i >= 0; --i) printf("%d ", moves[i]);
        printf("\n");
        free(moves);
    }

    free(dist); free(parent); free(parent_move); free(queue);
    return 0;
}

int main(void) {
    int mode = read_input_mode();
    if (mode == -1) return 1;

    int n = (mode == 1) ? read_manual_n() : generate_random_n();
    if (n < 0) return 1;

    return solve_security_switches(n);
}
