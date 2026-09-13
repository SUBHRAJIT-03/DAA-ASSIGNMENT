#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static unsigned neighbor_after_move(unsigned b, int n) {
    unsigned nb = 0;
    for (int i = 0; i < n; ++i) if (b & (1u << i)) {
        if (i > 0) nb |= 1u << (i - 1);
        if (i + 1 < n) nb |= 1u << (i + 1);
    }
    return nb;
}

static int max_depth_limit = 12;
static int visited_size = 0;
static char *memo = NULL;

static int dfs_belief(unsigned b, int n, int depth) {
    if (b == 0) return 1;
    if (depth > max_depth_limit) return 0;
    if (memo[b] != -1) return memo[b];

    for (int p = 0; p < n; ++p) {
        unsigned hit_mask = (b & (1u << p));
        (void)hit_mask;
        unsigned rem = b & ~(1u << p);
        unsigned next = neighbor_after_move(rem, n);
        if (dfs_belief(next, n, depth + 1)) {
            memo[b] = 1;
            return 1;
        }
    }
    memo[b] = 0;
    return 0;
}

static int read_input_mode(void) {
    int mode = 0;
    printf("Hitting a moving target — strategy explorer\n");
    printf("Choose input mode:\n 1) Manual n\n 2) Random n (small)\nEnter 1 or 2: ");
    if (scanf("%d", &mode) != 1) return -1;
    return mode;
}

static int read_manual_n(void) {
    int n = 0;
    printf("Enter number of hiding spots n (recommended <=12): ");
    if (scanf("%d", &n) != 1) return -1;
    return n;
}

static int generate_random_n(void) {
    int maxn = 10;
    printf("Enter max n for random generation (default 10): ");
    if (scanf("%d", &maxn) != 1) maxn = 10;
    if (maxn < 1) maxn = 10;
    srand((unsigned)time(NULL));
    int n = (rand() % maxn) + 1;
    printf("Random n = %d\n", n);
    return n;
}

static int solve_moving_target(int n) {
    if (n < 1 || n > 20) {
        printf("n must be between 1 and 20 for this tool.\n");
        return 1;
    }

    unsigned all = (n == 32) ? 0xFFFFFFFFu : ((1u << n) - 1u);
    visited_size = 1 << n;
    memo = malloc((size_t)visited_size * sizeof(char));
    if (!memo) {
        perror("malloc");
        return 1;
    }
    memset(memo, 0xFF, (size_t)visited_size * sizeof(char));

    int found = 0;
    for (max_depth_limit = 0; max_depth_limit <= 20; ++max_depth_limit) {
        memset(memo, 0xFF, (size_t)visited_size * sizeof(char));
        if (dfs_belief(all, n, 0)) {
            found = 1;
            break;
        }
    }

    if (found) printf("A guaranteed strategy exists within depth %d (search found).\n", max_depth_limit);
    else printf("No guaranteed strategy found up to depth 20 (may not exist).\n");

    free(memo);
    return 0;
}

int main(void) {
    int mode = read_input_mode();
    if (mode == -1) return 1;

    int n = (mode == 1) ? read_manual_n() : generate_random_n();
    if (n < 0) return 1;

    return solve_moving_target(n);
}
