#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

/* Frame-Stewart algorithm for r pegs. For r=4 (Reve's puzzle) we compute:
   T(0)=0, and for n>=1,
   T_r(n) = min_{1<=k<n} (2*T_r(k) + T_{r-1}(n-k))
   Base: T_3(n) = 2^n - 1.

   We compute T_4(n) using dynamic programming. n should be moderate (<=60)
   to avoid overflow in 64-bit integers.
*/

static int read_input_mode(void) {
    int mode = 0;
    printf("Reve's puzzle (4 pegs)\n");
    printf("Choose input mode:\n 1) Manual (enter number of disks n)\n 2) Random n (1..max)\nEnter 1 or 2: ");
    if (scanf("%d", &mode) != 1) return -1;
    return mode;
}

static int read_manual_n(void) {
    int n = 0;
    printf("Enter number of disks n (>=1, recommended <= 60): ");
    if (scanf("%d", &n) != 1) return -1;
    return n;
}

static int generate_random_n(void) {
    int maxn = 20;
    printf("Enter maximum n for random generation (default 20): ");
    if (scanf("%d", &maxn) != 1) maxn = 20;
    if (maxn < 1) maxn = 20;
    srand((unsigned)time(NULL));
    int n = (rand() % maxn) + 1;
    printf("Randomly chosen n = %d\n", n);
    return n;
}

static void solve_reve_puzzle(int n) {
    unsigned long long *T3 = calloc((size_t)n + 1, sizeof(unsigned long long));
    unsigned long long *T4 = calloc((size_t)n + 1, sizeof(unsigned long long));
    if (!T3 || !T4) {
        perror("calloc");
        free(T3);
        free(T4);
        return;
    }

    T3[0] = 0;
    for (int i = 1; i <= n; ++i) {
        if (i >= 64) T3[i] = ULLONG_MAX;
        else T3[i] = (1ULL << i) - 1ULL;
    }

    T4[0] = 0;
    for (int i = 1; i <= n; ++i) {
        unsigned long long best = ULLONG_MAX;
        for (int k = 1; k < i; ++k) {
            unsigned long long a = T4[k];
            unsigned long long b = T3[i - k];
            if (a == ULLONG_MAX || b == ULLONG_MAX) continue;
            if (a > ULLONG_MAX / 2) continue;
            unsigned long long cand = 2ULL * a + b;
            if (cand < best) best = cand;
        }
        if (i == 1) best = 1;
        T4[i] = best;
    }

    printf("n = %d disks\n", n);
    if (T4[n] == ULLONG_MAX) printf("Result too large to represent in 64-bit.\n");
    else printf("Minimum moves with 4 pegs (Reve) = %llu\n", T4[n]);

    free(T3);
    free(T4);
}

int main(void) {
    int mode = read_input_mode();
    if (mode == -1) return 1;

    int n = (mode == 1) ? read_manual_n() : generate_random_n();
    if (n < 1) {
        printf("n must be >=1\n");
        return 1;
    }

    solve_reve_puzzle(n);
    return 0;
}
