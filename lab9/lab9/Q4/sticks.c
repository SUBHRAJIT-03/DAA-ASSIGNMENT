/*
 * Minimum Cost to Connect Sticks
 * ------------------------------
 * Input representation
 *   The lengths are sorted once into an array (queue Q1).  Newly created
 *   sticks are appended to a second array (queue Q2).  Because the merged
 *   lengths are produced in non-decreasing order, Q2 is automatically
 *   sorted, so the two smallest sticks are always at the fronts of Q1/Q2.
 *   No priority queue is needed:   sort O(n log n),  merging O(n).
 *
 * Usage:  ./sticks < input.txt     ./sticks --test     ./sticks --bench
 * Input:  n
 *         L_1 L_2 ... L_n
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct { long long x, y; } Merge;

static int cmp_ll(const void *A, const void *B) {
    long long a = *(const long long *)A, b = *(const long long *)B;
    return (a > b) - (a < b);
}

/* Returns the minimum cost.  Optional outputs (may be NULL):
 *   mg[0..n-2]   the merges in order,
 *   sorted_out   the lengths sorted ascending,
 *   depth_out    depth of each sorted stick in the merge tree. */
static long long min_cost(const long long *L, int n, Merge *mg, long long *sorted_out, int *depth_out) {
    if (n <= 0) return 0;
    if (n == 1) { if (sorted_out) sorted_out[0] = L[0]; if (depth_out) depth_out[0] = 0; return 0; }
    int total = 2 * n - 1;
    long long *w = malloc(total * sizeof(long long));
    int *par = malloc(total * sizeof(int));
    memcpy(w, L, n * sizeof(long long));
    qsort(w, n, sizeof(long long), cmp_ll);
    if (sorted_out) memcpy(sorted_out, w, n * sizeof(long long));

    long long cost = 0;
    int i = 0, j = n, k = n, step = 0;            /* i: Q1 front, j: Q2 front, k: Q2 end */
    for (; k < total; k++, step++) {
        int c[2];
        for (int t = 0; t < 2; t++) {
            if (i < n && (j >= k || w[i] <= w[j])) c[t] = i++;
            else c[t] = j++;
        }
        w[k] = w[c[0]] + w[c[1]];
        cost += w[k];
        par[c[0]] = par[c[1]] = k;
        if (mg) { mg[step].x = w[c[0]]; mg[step].y = w[c[1]]; }
    }
    if (depth_out) {
        int *dep = malloc(total * sizeof(int));
        dep[total - 1] = 0;
        for (int v = total - 2; v >= 0; v--) dep[v] = dep[par[v]] + 1;
        for (int v = 0; v < n; v++) depth_out[v] = dep[v];
        free(dep);
    }
    free(w); free(par);
    return cost;
}

/* ---------------- baselines for validation ---------------- */
static long long naive_cost(const long long *L, int n) {            /* O(n^2): scan for two minima */
    long long *w = malloc((n + 1) * sizeof(long long)); int m = n;
    memcpy(w, L, n * sizeof(long long));
    long long cost = 0;
    while (m > 1) {
        int a = 0;
        for (int i = 1; i < m; i++) if (w[i] < w[a]) a = i;
        long long x = w[a]; w[a] = w[--m];
        int b = 0;
        for (int i = 1; i < m; i++) if (w[i] < w[b]) b = i;
        long long y = w[b]; w[b] = x + y; cost += x + y;
    }
    free(w);
    return cost;
}

static void hpush(long long *h, int *sz, long long x) {              /* binary min-heap */
    int i = (*sz)++;
    while (i > 0 && h[(i - 1) / 2] > x) { h[i] = h[(i - 1) / 2]; i = (i - 1) / 2; }
    h[i] = x;
}
static long long hpop(long long *h, int *sz) {
    long long top = h[0], x = h[--(*sz)]; int i = 0;
    for (;;) {
        int c = 2 * i + 1;
        if (c >= *sz) break;
        if (c + 1 < *sz && h[c + 1] < h[c]) c++;
        if (h[c] >= x) break;
        h[i] = h[c]; i = c;
    }
    if (*sz > 0) h[i] = x;
    return top;
}
static long long heap_cost(const long long *L, int n) {
    long long *h = malloc((n + 1) * sizeof(long long)); int sz = 0;
    for (int i = 0; i < n; i++) hpush(h, &sz, L[i]);
    long long cost = 0;
    while (sz > 1) { long long x = hpop(h, &sz), y = hpop(h, &sz); cost += x + y; hpush(h, &sz, x + y); }
    free(h);
    return cost;
}

static long long brute(const long long *a, int n) {                  /* every possible merge order, n <= 8 */
    if (n <= 1) return 0;
    long long best = (long long)4e18;
    for (int i = 0; i < n; i++) for (int j = i + 1; j < n; j++) {
        long long b[8]; int m = 0;
        for (int k = 0; k < n; k++) if (k != i && k != j) b[m++] = a[k];
        b[m++] = a[i] + a[j];
        long long c = a[i] + a[j] + brute(b, m);
        if (c < best) best = c;
    }
    return best;
}

/* replay the printed merges: each pair must exist in the current multiset */
static int replay_ok(const long long *L, int n, const Merge *mg, long long cost) {
    long long *a = malloc((n + 1) * sizeof(long long)); int m = n, ok = 1;
    memcpy(a, L, n * sizeof(long long));
    long long c = 0;
    for (int s = 0; s < n - 1 && ok; s++) {
        int ix = -1, iy = -1;
        for (int i = 0; i < m; i++) if (ix < 0 && a[i] == mg[s].x) ix = i;
        for (int i = 0; i < m; i++) if (i != ix && iy < 0 && a[i] == mg[s].y) iy = i;
        if (ix < 0 || iy < 0) { ok = 0; break; }
        long long z = a[ix] + a[iy];
        if (ix > iy) { int t = ix; ix = iy; iy = t; }
        a[iy] = a[--m]; if (ix == m) ix = iy;   /* remove iy, then overwrite ix */
        a[ix] = z; c += z;
    }
    if (ok && n >= 1) {
        long long sum = 0; for (int i = 0; i < n; i++) sum += L[i];
        if (m != 1 || a[0] != sum || c != cost) ok = 0;
    }
    free(a);
    return ok;
}

static int run_tests(void) {
    srand(77);
    int bad = 0, T1 = 600;
    for (int t = 0; t < T1; t++) {                                   /* brute force over all merge orders */
        int n = 1 + rand() % 8; long long L[8];
        for (int i = 0; i < n; i++) L[i] = rand() % (t % 2 ? 4 : 30);
        if (min_cost(L, n, NULL, NULL, NULL) != brute(L, n)) { bad++; printf("FAIL brute t=%d\n", t); }
    }
    printf("brute force (all merge orders, n<=8, ties/zeros) : %d/%d passed\n", T1 - bad, T1);
    int bad2 = 0, T2 = 500;
    for (int t = 0; t < T2; t++) {                                   /* baselines + structural checks */
        int n = 1 + rand() % 500; long long *L = malloc(n * sizeof(long long));
        for (int i = 0; i < n; i++) L[i] = 1 + rand() % (t % 3 ? 100000 : 5);
        Merge *mg = malloc(n * sizeof(Merge)); long long *srt = malloc(n * sizeof(long long)); int *dep = malloc(n * sizeof(int));
        long long c = min_cost(L, n, mg, srt, dep), nv = naive_cost(L, n), hp = heap_cost(L, n);
        long long viaDepth = 0; for (int i = 0; i < n; i++) viaDepth += srt[i] * dep[i];
        int ok = (c == nv) && (c == hp) && (c == viaDepth) && replay_ok(L, n, mg, c);
        if (n == 1) ok = ok && c == 0;
        if (!ok) { bad2++; printf("FAIL t=%d n=%d cost=%lld naive=%lld heap=%lld depth=%lld\n", t, n, c, nv, hp, viaDepth); }
        free(L); free(mg); free(srt); free(dep);
    }
    printf("naive O(n^2) + heap + sum(L*depth) + replay      : %d/%d passed\n", T2 - bad2, T2);
    long long eq[8] = {1, 1, 1, 1, 1, 1, 1, 1};
    long long fib[60]; fib[0] = 1; fib[1] = 1;
    for (int i = 2; i < 60; i++) fib[i] = fib[i - 1] + fib[i - 2];
    int ok3 = min_cost(eq, 8, NULL, NULL, NULL) == 24 && min_cost(fib, 60, NULL, NULL, NULL) == naive_cost(fib, 60)
              && min_cost(NULL, 0, NULL, NULL, NULL) == 0;
    printf("special cases (8 equal sticks = 24, Fibonacci, n=0): %s\n", ok3 ? "passed" : "FAILED");
    return bad + bad2 + !ok3;
}

static void bench(void) {
    srand(11);
    for (int n = 100000; n <= 1600000; n *= 2) {
        long long *L = malloc(n * sizeof(long long));
        for (int i = 0; i < n; i++) L[i] = 1 + rand() % 1000000;
        clock_t c0 = clock();
        long long cost = min_cost(L, n, NULL, NULL, NULL);
        double t_two = (double)(clock() - c0) / CLOCKS_PER_SEC;
        c0 = clock();
        long long hc = heap_cost(L, n);
        double t_heap = (double)(clock() - c0) / CLOCKS_PER_SEC;
        printf("n=%8d  two-queue=%.4f s  heap=%.4f s  %s\n", n, t_two, t_heap, cost == hc ? "(same cost)" : "(MISMATCH)");
        free(L);
    }
}

int main(int argc, char **argv) {
    if (argc > 1 && !strcmp(argv[1], "--test"))  return run_tests() ? 1 : 0;
    if (argc > 1 && !strcmp(argv[1], "--bench")) { bench(); return 0; }

    int n;
    if (scanf("%d", &n) != 1 || n < 0) { fprintf(stderr, "bad input\n"); return 1; }
    long long *L = malloc((n + 1) * sizeof(long long));
    for (int i = 0; i < n; i++)
        if (scanf("%lld", &L[i]) != 1 || L[i] < 0) { fprintf(stderr, "bad length %d\n", i + 1); return 1; }
    Merge *mg = malloc((n + 1) * sizeof(Merge));
    long long cost = min_cost(L, n, mg, NULL, NULL);
    printf("Minimum total cost = %lld\n", cost);
    for (int s = 0; s < n - 1; s++)
        printf("  merge %lld + %lld -> %lld (cost %lld)\n", mg[s].x, mg[s].y, mg[s].x + mg[s].y, mg[s].x + mg[s].y);
    free(L); free(mg);
    return 0;
}
