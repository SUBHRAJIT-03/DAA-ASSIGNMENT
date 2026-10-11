/*
 * Candy Distribution  (bi-directional slope greedy)
 * -------------------------------------------------
 * Input representation
 *   The ratings are one array r[0..n-1].  Only the SLOPE between neighbours
 *   matters (up / flat / down), so the line is viewed as runs of ascending
 *   and descending slopes.
 *      L[i] = length of the strictly increasing run of ratings ending at i
 *      R[i] = length of the strictly decreasing run of ratings starting at i
 *   candy[i] = max(L[i], R[i])   (computed by a left-to-right and a
 *   right-to-left pass, using one array c[]).   Time O(n), space O(n).
 *   A one-pass version that keeps only (up, down, peak) uses O(1) extra space.
 *
 * Usage:  ./candy < input.txt     ./candy --test     ./candy --bench
 * Input:  n
 *         r_1 r_2 ... r_n
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---- main algorithm: two passes, fills c[] and returns the total ---- */
static long long candy_two_pass(const long long *r, int n, int *c) {
    if (n <= 0) return 0;
    for (int i = 0; i < n; i++) c[i] = 1;
    for (int i = 1; i < n; i++)                       /* left -> right : L[i] */
        if (r[i] > r[i - 1]) c[i] = c[i - 1] + 1;
    for (int i = n - 2; i >= 0; i--)                  /* right -> left : max(L[i], R[i]) */
        if (r[i] > r[i + 1] && c[i] < c[i + 1] + 1) c[i] = c[i + 1] + 1;
    long long total = 0;
    for (int i = 0; i < n; i++) total += c[i];
    return total;
}

/* ---- one-pass slope version, O(1) extra space ---- */
static long long candy_one_pass(const long long *r, int n) {
    if (n <= 0) return 0;
    long long total = 1; long long up = 0, down = 0, peak = 0;
    for (int i = 1; i < n; i++) {
        if (r[i] > r[i - 1]) { up++; down = 0; peak = up; total += 1 + up; }
        else if (r[i] == r[i - 1]) { up = down = peak = 0; total += 1; }
        else { up = 0; down++; total += down + (peak >= down ? 0 : 1); }
    }
    return total;
}

/* ---------------- baselines for validation ---------------- */
static const long long *g_r;
static int cmp_rating(const void *A, const void *B) {
    long long a = g_r[*(const int *)A], b = g_r[*(const int *)B];
    return (a > b) - (a < b);
}
/* DAG longest path: handle children in increasing rating order, c[i] = 1 + max c[lower neighbour] */
static long long candy_sorted(const long long *r, int n) {
    if (n <= 0) return 0;
    int *id = malloc(n * sizeof(int)), *c = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) id[i] = i;
    g_r = r; qsort(id, n, sizeof(int), cmp_rating);
    long long total = 0;
    for (int k = 0; k < n; k++) {
        int i = id[k], best = 0;
        if (i > 0 && r[i - 1] < r[i] && c[i - 1] > best) best = c[i - 1];
        if (i + 1 < n && r[i + 1] < r[i] && c[i + 1] > best) best = c[i + 1];
        c[i] = best + 1; total += c[i];
    }
    free(id); free(c);
    return total;
}

static int valid(const long long *r, int n, const int *c) {
    for (int i = 0; i < n; i++) if (c[i] < 1) return 0;
    for (int i = 0; i + 1 < n; i++) {
        if (r[i] > r[i + 1] && c[i] <= c[i + 1]) return 0;
        if (r[i] < r[i + 1] && c[i] >= c[i + 1]) return 0;
    }
    return 1;
}

static long long bf_best; static int bf_c[10];
static void dfs(const long long *r, int n, int i, long long sum) {      /* exhaustive, n <= 8 */
    if (sum >= bf_best) return;
    if (i == n) { bf_best = sum; return; }
    for (int v = 1; v <= n; v++) {
        if (i > 0) {
            if (r[i] > r[i - 1] && v <= bf_c[i - 1]) continue;
            if (r[i] < r[i - 1] && v >= bf_c[i - 1]) continue;
        }
        bf_c[i] = v; dfs(r, n, i + 1, sum + v);
    }
}
static long long brute(const long long *r, int n) { bf_best = (long long)4e18; dfs(r, n, 0, 0); return bf_best; }

static int run_tests(void) {
    srand(31337);
    int bad = 0, T1 = 1500;
    for (int t = 0; t < T1; t++) {                       /* brute force: all assignments */
        int n = 1 + rand() % 8; long long r[8]; int c[8];
        for (int i = 0; i < n; i++) r[i] = rand() % (t % 2 ? 3 : 6);
        long long a = candy_two_pass(r, n, c), b = brute(r, n), d = candy_one_pass(r, n);
        if (a != b || d != b || !valid(r, n, c)) { bad++; printf("FAIL brute t=%d two=%lld one=%lld brute=%lld\n", t, a, d, b); }
    }
    printf("brute force (all assignments, n<=8, many ties) : %d/%d passed\n", T1 - bad, T1);
    int bad2 = 0, T2 = 1000;
    for (int t = 0; t < T2; t++) {                       /* three-way comparison */
        int n = 1 + rand() % 2000; long long *r = malloc(n * sizeof(long long)); int *c = malloc(n * sizeof(int));
        int mode = t % 4;
        for (int i = 0; i < n; i++) r[i] = mode == 0 ? rand() % 1000000 : mode == 1 ? rand() % 4 : mode == 2 ? (i % 50) : (long long)(i / 7) * ((i / 7) % 2 ? -1 : 1);
        long long a = candy_two_pass(r, n, c), d = candy_one_pass(r, n), s = candy_sorted(r, n);
        if (a != d || a != s || !valid(r, n, c)) { bad2++; printf("FAIL t=%d n=%d two=%lld one=%lld sorted=%lld\n", t, n, a, d, s); }
        free(r); free(c);
    }
    printf("two-pass = one-pass = rating-order DAG baseline : %d/%d passed\n", T2 - bad2, T2);
    int n = 1000, ok = 1; long long *r = malloc(n * sizeof(long long)); int *c = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) r[i] = i;
    ok &= candy_two_pass(r, n, c) == (long long)n * (n + 1) / 2;
    for (int i = 0; i < n; i++) r[i] = n - i;
    ok &= candy_two_pass(r, n, c) == (long long)n * (n + 1) / 2;
    for (int i = 0; i < n; i++) r[i] = 7;
    ok &= candy_two_pass(r, n, c) == n;
    for (int i = 0; i < n; i++) r[i] = i % 2;
    ok &= candy_two_pass(r, n, c) == (long long)n / 2 * 3;
    ok &= candy_two_pass(r, 1, c) == 1 && candy_two_pass(r, 0, c) == 0;
    printf("special cases (up, down, flat, zigzag, n=1, n=0)  : %s\n", ok ? "passed" : "FAILED");
    free(r); free(c);
    return bad + bad2 + !ok;
}

static void bench(void) {
    srand(8);
    for (int n = 400000; n <= 6400000; n *= 2) {
        long long *r = malloc(n * sizeof(long long)); int *c = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) r[i] = rand() % 1000000;
        clock_t c0 = clock(); long long a = candy_two_pass(r, n, c);
        double t1 = (double)(clock() - c0) / CLOCKS_PER_SEC;
        c0 = clock(); long long b = candy_one_pass(r, n);
        double t2 = (double)(clock() - c0) / CLOCKS_PER_SEC;
        printf("n=%8d  two-pass=%.4f s  one-pass=%.4f s  %s\n", n, t1, t2, a == b ? "(same total)" : "(MISMATCH)");
        free(r); free(c);
    }
}

int main(int argc, char **argv) {
    if (argc > 1 && !strcmp(argv[1], "--test"))  return run_tests() ? 1 : 0;
    if (argc > 1 && !strcmp(argv[1], "--bench")) { bench(); return 0; }

    int n;
    if (scanf("%d", &n) != 1 || n < 0) { fprintf(stderr, "bad input\n"); return 1; }
    long long *r = malloc((n + 1) * sizeof(long long)); int *c = malloc((n + 1) * sizeof(int));
    for (int i = 0; i < n; i++) if (scanf("%lld", &r[i]) != 1) { fprintf(stderr, "bad rating %d\n", i + 1); return 1; }
    long long total = candy_two_pass(r, n, c);
    printf("Minimum total candies = %lld\n", total);
    printf("%-8s %-12s %s\n", "child", "rating", "candies");
    for (int i = 0; i < n; i++) printf("%-8d %-12lld %d\n", i + 1, r[i], c[i]);
    free(r); free(c);
    return 0;
}
