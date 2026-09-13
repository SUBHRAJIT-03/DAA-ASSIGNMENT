#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

static int read_input_mode(void) {
    int mode = 0;
    printf("Matrix Chain Multiplication (MCM)\n");
    printf("Choose input mode:\n 1) Manual (enter n and dimensions)\n 2) Random (generate dims)\nEnter 1 or 2: ");
    if (scanf("%d", &mode) != 1) return -1;
    return mode;
}

static int read_manual_dimensions(int *n, int **p) {
    printf("Enter number of matrices n: ");
    if (scanf("%d", n) != 1 || *n < 1) return 0;

    *p = malloc(sizeof(int) * (*n + 1));
    if (!*p) {
        perror("malloc");
        return 0;
    }

    printf("Enter %d+1 dimensions p0 p1 ... p%d: ", *n, *n);
    for (int i = 0; i <= *n; ++i) {
        if (scanf("%d", &(*p)[i]) != 1) {
            free(*p);
            *p = NULL;
            return 0;
        }
    }
    return 1;
}

static int generate_random_dimensions(int *n, int **p) {
    int maxn = 6;
    printf("Enter max n for random generation (default 6): ");
    if (scanf("%d", &maxn) != 1) maxn = 6;
    if (maxn < 1) maxn = 6;

    srand((unsigned)time(NULL));
    *n = (rand() % maxn) + 1;
    *p = malloc(sizeof(int) * (*n + 1));
    if (!*p) {
        perror("malloc");
        return 0;
    }

    for (int i = 0; i <= *n; ++i) (*p)[i] = (rand() % 10) + 1;
    printf("Random n=%d dimensions: ", *n);
    for (int i = 0; i <= *n; ++i) printf("%d ", (*p)[i]);
    printf("\n");
    return 1;
}

static void print_opt(int i, int j, int **s) {
    if (i == j) {
        printf("A%d", i);
        return;
    }
    int k = s[i][j];
    printf("(");
    print_opt(i, k, s);
    printf(" x ");
    print_opt(k + 1, j, s);
    printf(")");
}

static long long solve_mcm(int n, const int *p) {
    long long **m = malloc((size_t)(n + 1) * sizeof(long long *));
    if (!m) {
        perror("malloc");
        return -1;
    }

    int **s = malloc((size_t)(n + 1) * sizeof(int *));
    if (!s) {
        perror("malloc");
        free(m);
        return -1;
    }

    for (int i = 0; i <= n; ++i) {
        m[i] = calloc((size_t)(n + 1), sizeof(long long));
        s[i] = calloc((size_t)(n + 1), sizeof(int));
        if (!m[i] || !s[i]) {
            perror("calloc");
            for (int j = 0; j < i; ++j) { free(m[j]); free(s[j]); }
            free(m); free(s);
            return -1;
        }
    }

    for (int i = 1; i <= n; ++i) m[i][i] = 0;
    for (int L = 2; L <= n; ++L) {
        for (int i = 1; i <= n - L + 1; ++i) {
            int j = i + L - 1;
            m[i][j] = LLONG_MAX;
            for (int k = i; k <= j - 1; ++k) {
                long long q = m[i][k] + m[k + 1][j] + (long long)p[i - 1] * p[k] * p[j];
                if (q < m[i][j]) {
                    m[i][j] = q;
                    s[i][j] = k;
                }
            }
        }
    }

    long long result = m[1][n];
    printf("Minimum number of scalar multiplications = %lld\n", result);
    printf("Optimal parenthesization (recursive):\n");
    print_opt(1, n, s);
    printf("\n");

    for (int i = 0; i <= n; ++i) {
        free(m[i]);
        free(s[i]);
    }
    free(m);
    free(s);

    return result;
}

int main(void) {
    int mode = read_input_mode();
    if (mode == -1) return 1;

    int n = 0;
    int *p = NULL;

    if (mode == 1) {
        if (!read_manual_dimensions(&n, &p)) return 1;
    } else {
        if (!generate_random_dimensions(&n, &p)) return 1;
    }

    if (n < 1) {
        printf("n must be >=1\n");
        free(p);
        return 1;
    }

    long long result = solve_mcm(n, p);
    (void)result;

    free(p);
    return 0;
}
