/*
 * Minimise Deviation in Array  (two-way greedy with heaps)
 * --------------------------------------------------------
 * Operations: odd x -> 2x,  even x -> x/2.   Deviation = max(A) - min(A).
 *
 * Input representation (normal form)
 *   Write a_i = o_i * 2^e_i with o_i odd.  The values reachable from a_i are
 *        { o_i * 2^k : 0 <= k <= kmax_i },   kmax_i = max(e_i, 1)
 *   (an odd number can be doubled once, an even number can be halved down to o_i,
 *   an even number can never be doubled).  So element i is a CHAIN of candidate
 *   values, stored as the pair (o_i, kmax_i), and the problem becomes:
 *   pick one value from every chain so that max - min is smallest.
 *
 *   Down-greedy (max-heap): start every element at its top value o*2^kmax,
 *       repeatedly lower the current maximum by one step; stop when it cannot.
 *   Up-greedy (min-heap):   start every element at o_i, repeatedly raise the
 *       current minimum by one step; stop when it cannot.
 *
 * Usage:  ./deviation < input.txt    ./deviation --test    ./deviation --bench
 * Input:  n
 *         a_1 ... a_n      (positive integers)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct { long long o; int kmax; } El;
typedef struct { long long key; int idx; } HE;      /* max-heap on key */

static void hpush(HE *h, int *sz, HE x) {
    int i = (*sz)++;
    while (i > 0 && h[(i - 1) / 2].key < x.key) { h[i] = h[(i - 1) / 2]; i = (i - 1) / 2; }
    h[i] = x;
}
static HE hpop(HE *h, int *sz) {
    HE top = h[0], x = h[--(*sz)]; int i = 0;
    for (;;) {
        int c = 2 * i + 1;
        if (c >= *sz) break;
        if (c + 1 < *sz && h[c + 1].key > h[c].key) c++;
        if (h[c].key <= x.key) break;
        h[i] = h[c]; i = c;
    }
    if (*sz > 0) h[i] = x;
    return top;
}

static void normalise(const long long *a, int n, El *e) {
    for (int i = 0; i < n; i++) {
        long long o = a[i]; int ex = 0;
        while ((o & 1) == 0) { o >>= 1; ex++; }
        e[i].o = o; e[i].kmax = ex > 1 ? ex : 1;
    }
}

/* ---- down-greedy with a max-heap; returns min deviation and the best max value ---- */
static long long dev_down(const El *e, int n, long long *best_max) {
    HE *h = malloc((n + 1) * sizeof(HE)); int sz = 0;
    int *lvl = malloc(n * sizeof(int));
    long long mn = (long long)4e18;
    for (int i = 0; i < n; i++) {
        lvl[i] = e[i].kmax;
        long long v = e[i].o << lvl[i];
        HE x = { v, i }; hpush(h, &sz, x);
        if (v < mn) mn = v;
    }
    long long best = h[0].key - mn, bm = h[0].key;
    for (;;) {
        HE t = hpop(h, &sz);
        if (lvl[t.idx] == 0) break;                    /* the maximum cannot be lowered any more */
        lvl[t.idx]--;
        long long v = e[t.idx].o << lvl[t.idx];
        HE x = { v, t.idx }; hpush(h, &sz, x);
        if (v < mn) mn = v;
        if (h[0].key - mn < best) { best = h[0].key - mn; bm = h[0].key; }
    }
    free(h); free(lvl);
    if (best_max) *best_max = bm;
    return best;
}

/* ---- up-greedy with a min-heap (keys negated) ---- */
static long long dev_up(const El *e, int n) {
    HE *h = malloc((n + 1) * sizeof(HE)); int sz = 0;
    int *lvl = calloc(n, sizeof(int));
    long long mx = 0;
    for (int i = 0; i < n; i++) {
        HE x = { -e[i].o, i }; hpush(h, &sz, x);
        if (e[i].o > mx) mx = e[i].o;
    }
    long long best = mx + h[0].key;                     /* mx - min */
    for (;;) {
        HE t = hpop(h, &sz);
        if (lvl[t.idx] == e[t.idx].kmax) break;         /* the minimum cannot be raised any more */
        lvl[t.idx]++;
        long long v = e[t.idx].o << lvl[t.idx];
        HE x = { -v, t.idx }; hpush(h, &sz, x);
        if (v > mx) mx = v;
        if (mx + h[0].key < best) best = mx + h[0].key;
    }
    free(h); free(lvl);
    return best;
}

/* final array: largest reachable value <= best_max for every element */
static void final_array(const El *e, int n, long long best_max, long long *out) {
    for (int i = 0; i < n; i++)
        for (int k = e[i].kmax; k >= 0; k--)
            if ((e[i].o << k) <= best_max) { out[i] = e[i].o << k; break; }
}

/* ---------------- baselines for validation ---------------- */
typedef struct { long long v; int id; } VP;
static int cmp_vp(const void *A, const void *B) {
    const VP *p = A, *q = B;
    return (p->v > q->v) - (p->v < q->v);
}
/* smallest range containing one candidate of every chain (sort all candidates + two pointers) */
static long long dev_window(const El *e, int n) {
    int N = 0;
    for (int i = 0; i < n; i++) N += e[i].kmax + 1;
    VP *p = malloc(N * sizeof(VP)); int m = 0;
    for (int i = 0; i < n; i++) for (int k = 0; k <= e[i].kmax; k++) { p[m].v = e[i].o << k; p[m].id = i; m++; }
    qsort(p, N, sizeof(VP), cmp_vp);
    int *cnt = calloc(n, sizeof(int)), covered = 0, l = 0;
    long long best = (long long)4e18;
    for (int r = 0; r < N; r++) {
        if (cnt[p[r].id]++ == 0) covered++;
        while (covered == n) {
            if (p[r].v - p[l].v < best) best = p[r].v - p[l].v;
            if (--cnt[p[l].id] == 0) covered--;
            l++;
        }
    }
    free(p); free(cnt);
    return best;
}

static long long bf_best;                               /* enumerate one candidate per element */
static void bf(const El *e, int n, int i, long long mn, long long mx) {
    if (i == n) { if (mx - mn < bf_best) bf_best = mx - mn; return; }
    for (int k = 0; k <= e[i].kmax; k++) {
        long long v = e[i].o << k;
        bf(e, n, i + 1, v < mn ? v : mn, v > mx ? v : mx);
    }
}

/* ground truth by BFS on the REAL operations (odd -> 2x, even -> x/2); n <= 4.
   Also checks the reachable-set claim: every visited value is o_i * 2^k with k <= kmax_i. */
static long long bfs_truth(const long long *a, int n, int *model_ok) {
    El e[4]; normalise(a, n, e);
    static char seen[1 << 12];
    memset(seen, 0, sizeof seen);
    long long (*q)[4] = malloc(sizeof(long long[4]) * 4096);
    int qh = 0, qt = 0; long long best = (long long)4e18;
    int code0 = 0, mul = 1;
    long long start[4];
    for (int i = 0; i < n; i++) {
        start[i] = a[i]; int k = 0; long long r = a[i] / e[i].o; while (r > 1) { r >>= 1; k++; }
        code0 += k * mul; mul *= 8;
    }
    memcpy(q[qt++], start, sizeof start); seen[code0] = 1;
    while (qh < qt) {
        long long cur[4]; memcpy(cur, q[qh++], sizeof cur);
        long long mn = cur[0], mx = cur[0];
        for (int i = 1; i < n; i++) { if (cur[i] < mn) mn = cur[i]; if (cur[i] > mx) mx = cur[i]; }
        if (mx - mn < best) best = mx - mn;
        for (int i = 0; i < n; i++) {
            long long nxt[4]; memcpy(nxt, cur, sizeof nxt);
            nxt[i] = (cur[i] & 1) ? cur[i] * 2 : cur[i] / 2;
            int code = 0, m2 = 1, ok = 1;
            for (int j = 0; j < n; j++) {
                long long r = nxt[j] / e[j].o; int k = 0;
                if (nxt[j] % e[j].o != 0 || (r & (r - 1)) != 0) ok = 0;
                while (r > 1) { r >>= 1; k++; }
                if (k > e[j].kmax) ok = 0;
                code += k * m2; m2 *= 8;
            }
            if (!ok) { *model_ok = 0; continue; }
            if (!seen[code]) { seen[code] = 1; memcpy(q[qt++], nxt, sizeof nxt); }
        }
    }
    free(q);
    return best;
}

static int reachable(const El *e, long long v) {
    for (int k = 0; k <= e->kmax; k++) if ((e->o << k) == v) return 1;
    return 0;
}

static int run_tests(void) {
    srand(2718);
    int bad = 0, T1 = 1500, model_ok = 1;
    for (int t = 0; t < T1; t++) {                          /* BFS on the real operations */
        int n = 1 + rand() % 4; long long a[4]; El e[4];
        for (int i = 0; i < n; i++) a[i] = 1 + rand() % 24;
        normalise(a, n, e);
        long long truth = bfs_truth(a, n, &model_ok);
        bf_best = (long long)4e18; bf(e, n, 0, (long long)4e18, 0);
        long long d1 = dev_down(e, n, NULL), d2 = dev_up(e, n), d3 = dev_window(e, n);
        if (truth != d1 || truth != d2 || truth != d3 || truth != bf_best) {
            bad++; printf("FAIL bfs t=%d truth=%lld down=%lld up=%lld window=%lld brute=%lld\n", t, truth, d1, d2, d3, bf_best);
        }
    }
    printf("BFS on real operations (n<=4, values<=24)         : %d/%d passed, reachable-set model %s\n", T1 - bad, T1, model_ok ? "confirmed" : "VIOLATED");
    int bad2 = 0, T2 = 1000;
    for (int t = 0; t < T2; t++) {                          /* larger inputs, three algorithms */
        int n = 1 + rand() % 300; long long mod = t % 3 == 0 ? 1000000000LL : t % 3 == 1 ? 10000 : 64;
        long long *a = malloc(n * sizeof(long long)), *f = malloc(n * sizeof(long long)); El *e = malloc(n * sizeof(El));
        for (int i = 0; i < n; i++) a[i] = 1 + (((long long)rand() << 15) ^ rand()) % mod;
        if (t % 10 == 0) for (int i = 0; i < n; i++) a[i] = 1LL << (rand() % 30);
        normalise(a, n, e);
        long long bm, d1 = dev_down(e, n, &bm), d2 = dev_up(e, n), d3 = dev_window(e, n);
        final_array(e, n, bm, f);
        long long mn = f[0], mx = f[0]; int ok = (d1 == d2 && d1 == d3);
        for (int i = 0; i < n; i++) { if (!reachable(&e[i], f[i])) ok = 0; if (f[i] < mn) mn = f[i]; if (f[i] > mx) mx = f[i]; }
        if (mx - mn != d1) ok = 0;
        if (!ok) { bad2++; printf("FAIL large t=%d n=%d down=%lld up=%lld window=%lld final=%lld\n", t, n, d1, d2, d3, mx - mn); }
        free(a); free(f); free(e);
    }
    printf("down = up = window, final array valid (n<=300)    : %d/%d passed\n", T2 - bad2, T2);
    int ok = 1; El e[8]; long long x;
    long long t1[] = {1, 2, 3, 4}, t2[] = {4, 1, 5, 20, 3}, t3[] = {2, 10, 8}, t4[] = {5}, t5[] = {7, 7, 7}, t6[] = {1073741824, 1};
    normalise(t1, 4, e); ok &= dev_down(e, 4, &x) == 1;
    normalise(t2, 5, e); ok &= dev_down(e, 5, &x) == 3;
    normalise(t3, 3, e); ok &= dev_down(e, 3, &x) == 3;
    normalise(t4, 1, e); ok &= dev_down(e, 1, &x) == 0;
    normalise(t5, 3, e); ok &= dev_down(e, 3, &x) == 0;
    normalise(t6, 2, e); ok &= dev_down(e, 2, &x) == dev_window(e, 2) && dev_up(e, 2) == dev_window(e, 2);
    printf("special cases (known answers, n=1, equal, 2^30)    : %s\n", ok ? "passed" : "FAILED");
    return bad + bad2 + !ok + !model_ok;
}

static double now(void) { return (double)clock() / CLOCKS_PER_SEC; }
static void bench(void) {
    srand(9);
    printf("-- growth in n (values up to 1e9) --\n");
    for (int n = 50000; n <= 800000; n *= 2) {
        long long *a = malloc(n * sizeof(long long)); El *e = malloc(n * sizeof(El));
        for (int i = 0; i < n; i++) a[i] = 1 + (((long long)rand() << 15) ^ rand()) % 1000000000LL;
        normalise(a, n, e);
        double t0 = now(); long long d1 = dev_down(e, n, NULL); double t1 = now() - t0;
        t0 = now(); long long d2 = dev_up(e, n); double t2 = now() - t0;
        printf("n=%7d  down=%.4f s  up=%.4f s  %s (deviation %lld)\n", n, t1, t2, d1 == d2 ? "same" : "MISMATCH", d1);
        free(a); free(e);
    }
    printf("-- growth in the value range (n = 200 000) --\n");
    int n = 200000;
    for (int bits = 4; bits <= 30; bits += 13) {
        long long *a = malloc(n * sizeof(long long)); El *e = malloc(n * sizeof(El));
        for (int i = 0; i < n; i++) { long long o = 1 + 2 * (rand() % 1000); a[i] = o << (rand() % (bits - 9 > 0 ? bits - 9 : 1)); }
        normalise(a, n, e);
        double t0 = now(); long long d1 = dev_down(e, n, NULL); double t1 = now() - t0;
        printf("up to ~2^%-2d  down=%.4f s  (deviation %lld)\n", bits + 1, t1, d1);
        free(a); free(e);
    }
}

int main(int argc, char **argv) {
    if (argc > 1 && !strcmp(argv[1], "--test"))  return run_tests() ? 1 : 0;
    if (argc > 1 && !strcmp(argv[1], "--bench")) { bench(); return 0; }

    int n;
    if (scanf("%d", &n) != 1 || n <= 0) { fprintf(stderr, "bad input\n"); return 1; }
    long long *a = malloc(n * sizeof(long long)), *f = malloc(n * sizeof(long long)); El *e = malloc(n * sizeof(El));
    for (int i = 0; i < n; i++)
        if (scanf("%lld", &a[i]) != 1 || a[i] <= 0) { fprintf(stderr, "bad element %d\n", i + 1); return 1; }
    normalise(a, n, e);
    long long bm, dev = dev_down(e, n, &bm);
    final_array(e, n, bm, f);
    printf("Minimum deviation = %lld\n", dev);
    printf("%-8s %-12s %s\n", "index", "original", "final");
    for (int i = 0; i < n; i++) printf("%-8d %-12lld %lld\n", i + 1, a[i], f[i]);
    free(a); free(f); free(e);
    return 0;
}
