/*
 * Minimum Refuelling Stops  (+ Minimum Initial Fuel by reverse greedy)
 * --------------------------------------------------------------------
 * Input representation
 *   Stations are pairs (d_i, f_i) in an array sorted by distance.
 *   The state of the journey is a single number  reach = F + (sum of fuel
 *   taken so far)  = the farthest point we can get to.  The stations we
 *   have already passed (d_i <= reach) but not used are kept in a MAX-HEAP
 *   keyed by f_i  ("refuel retroactively").
 *
 *   Stops greedy: while reach < D: push every station with d <= reach;
 *                 if heap empty -> impossible; else pop the largest f.
 *   Initial-fuel reverse greedy: scan stations from the target back to the
 *                 origin keeping R = fuel needed on arrival; O(n) after sort.
 *
 * Usage:  ./fuel < input.txt     ./fuel --test     ./fuel --bench
 * Input:  D F n
 *         d_1 f_1
 *         ...
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct { long long d, f; } St;
typedef struct { long long f; int idx; } HE;

static int cmp_st(const void *A, const void *B) {
    const St *p = A, *q = B;
    if (p->d != q->d) return p->d < q->d ? -1 : 1;
    return p->f < q->f ? -1 : p->f > q->f;
}

/* ---- binary max-heap ---- */
static void hpush(HE *h, int *sz, HE x) {
    int i = (*sz)++;
    while (i > 0 && h[(i - 1) / 2].f < x.f) { h[i] = h[(i - 1) / 2]; i = (i - 1) / 2; }
    h[i] = x;
}
static HE hpop(HE *h, int *sz) {
    HE top = h[0], x = h[--(*sz)];
    int i = 0;
    for (;;) {
        int c = 2 * i + 1;
        if (c >= *sz) break;
        if (c + 1 < *sz && h[c + 1].f > h[c].f) c++;
        if (h[c].f <= x.f) break;
        h[i] = h[c]; i = c;
    }
    if (*sz > 0) h[i] = x;
    return top;
}

/* st sorted by d.  Returns min #stops or -1.  chosen[] receives station indices. */
static int min_stops(const St *st, int n, long long D, long long F, int *chosen) {
    HE *heap = malloc((n + 1) * sizeof(HE));
    int sz = 0, i = 0, stops = 0;
    long long reach = F;
    while (reach < D) {
        while (i < n && st[i].d <= reach && st[i].d < D) { HE e = { st[i].f, i }; hpush(heap, &sz, e); i++; }
        if (sz == 0) { free(heap); return -1; }
        HE b = hpop(heap, &sz);
        reach += b.f;
        if (chosen) chosen[stops] = b.idx;
        stops++;
    }
    free(heap);
    return stops;
}

/* Reverse greedy: smallest F for which the target is reachable at all (stops unlimited). */
static long long min_initial_fuel(const St *st, int n, long long D) {
    long long R = 0, next = D;           /* R = fuel needed when ARRIVING at `next` */
    for (int j = n - 1; j >= 0; j--) {
        if (st[j].d >= D) continue;      /* stations at/after the target are useless */
        long long need = (next - st[j].d) + R - st[j].f;
        R = need > 0 ? need : 0;
        next = st[j].d;
    }
    return next + R;                     /* from the origin (no refuel there) */
}

/* ---------------- validation ---------------- */
static int brute(const St *st, int n, long long D, long long F) {   /* n <= 16: all subsets */
    int best = -1;
    for (int mask = 0; mask < (1 << n); mask++) {
        long long fuel = F, pos = 0; int ok = 1, cnt = 0;
        for (int i = 0; i < n && ok; i++) if (mask >> i & 1) {
            if (st[i].d >= D) { ok = 0; break; }
            if (st[i].d - pos > fuel) { ok = 0; break; }
            fuel -= st[i].d - pos; pos = st[i].d; fuel += st[i].f; cnt++;
        }
        if (ok && D - pos <= fuel && (best < 0 || cnt < best)) best = cnt;
    }
    return best;
}
static int dp_stops(const St *st, int n, long long D, long long F) { /* O(n^2): dp[k] = max reach with k stops */
    long long *dp = calloc(n + 2, sizeof(long long));
    for (int k = 0; k <= n; k++) dp[k] = -1;
    dp[0] = F;
    for (int i = 0; i < n; i++)
        for (int k = i; k >= 0; k--)
            if (dp[k] >= st[i].d && dp[k] + st[i].f > dp[k + 1]) dp[k + 1] = dp[k] + st[i].f;
    int ans = -1;
    for (int k = 0; k <= n; k++) if (dp[k] >= D) { ans = k; break; }
    free(dp);
    return ans;
}
static int valid_plan(const St *st, int n, long long D, long long F, const int *ch, int k) {
    char *used = calloc(n + 1, 1);
    for (int i = 0; i < k; i++) { if (ch[i] < 0 || ch[i] >= n || used[ch[i]]) { free(used); return 0; } used[ch[i]] = 1; }
    long long fuel = F, pos = 0; int ok = 1;
    for (int i = 0; i < n && ok; i++) if (used[i]) {
        if (st[i].d - pos > fuel) ok = 0; else { fuel -= st[i].d - pos; pos = st[i].d; fuel += st[i].f; }
    }
    if (ok && D - pos > fuel) ok = 0;
    free(used);
    return ok;
}

static void gen(St *st, int n, long long *D, long long *F, int dense) {
    long long span = dense ? 4 * n + 4 : 60;
    *D = 1 + rand() % (span + 20);
    for (int i = 0; i < n; i++) { st[i].d = rand() % (*D + 5); st[i].f = 1 + rand() % (dense ? 10 : 25); }
    *F = rand() % (dense ? 15 : 30);
    qsort(st, n, sizeof(St), cmp_st);
}

static int run_tests(void) {
    srand(99);
    int bad = 0, T1 = 1500;
    for (int t = 0; t < T1; t++) {                         /* brute force */
        int n = rand() % 13; St st[16]; long long D, F; int ch[16];
        gen(st, n, &D, &F, t % 2);
        int g = min_stops(st, n, D, F, ch), b = brute(st, n, D, F);
        if (g != b || (g >= 0 && !valid_plan(st, n, D, F, ch, g))) { bad++; printf("FAIL brute t=%d greedy=%d brute=%d\n", t, g, b); }
    }
    printf("brute force (all subsets, n<=12) + plan check : %d/%d passed\n", T1 - bad, T1);
    int bad2 = 0, T2 = 600;
    for (int t = 0; t < T2; t++) {                         /* O(n^2) DP baseline */
        int n = 1 + rand() % 300; St *st = malloc(n * sizeof(St)); long long D, F; int *ch = malloc(n * sizeof(int));
        gen(st, n, &D, &F, 1);
        int g = min_stops(st, n, D, F, ch), d = dp_stops(st, n, D, F);
        if (g != d || (g >= 0 && !valid_plan(st, n, D, F, ch, g))) { bad2++; printf("FAIL dp t=%d greedy=%d dp=%d\n", t, g, d); }
        free(st); free(ch);
    }
    printf("DP baseline max-reach[k] (n<=300)             : %d/%d passed\n", T2 - bad2, T2);
    int bad3 = 0, T3 = 1500;
    for (int t = 0; t < T3; t++) {                         /* reverse greedy for initial fuel */
        int n = rand() % 60; St *st = malloc((n + 1) * sizeof(St)); long long D, F;
        gen(st, n, &D, &F, t % 2);
        long long Fm = min_initial_fuel(st, n, D);
        int a = min_stops(st, n, D, Fm, NULL);
        int b = Fm > 0 ? min_stops(st, n, D, Fm - 1, NULL) : -2;
        if (a < 0 || (Fm > 0 && b != -1)) { bad3++; printf("FAIL minF t=%d Fmin=%lld a=%d b=%d\n", t, Fm, a, b); }
        free(st);
    }
    printf("reverse greedy min fuel: feasible at F*, not at F*-1 : %d/%d passed\n", T3 - bad3, T3);
    return bad + bad2 + bad3;
}

static void bench(void) {
    srand(3);
    for (int n = 100000; n <= 1600000; n *= 2) {
        St *st = malloc(n * sizeof(St)); int *ch = malloc((n + 1) * sizeof(int));
        for (int i = 0; i < n; i++) { st[i].d = (long long)(i + 1) * 10; st[i].f = 10 + rand() % 21; }
        for (int i = n - 1; i > 0; i--) { int j = rand() % (i + 1); St t = st[i]; st[i] = st[j]; st[j] = t; }
        clock_t c0 = clock();
        qsort(st, n, sizeof(St), cmp_st);
        int r = min_stops(st, n, (long long)(n + 1) * 10, 10, ch);
        printf("n=%8d  stops=%d  time=%.4f s\n", n, r, (double)(clock() - c0) / CLOCKS_PER_SEC);
        free(st); free(ch);
    }
}

int main(int argc, char **argv) {
    if (argc > 1 && !strcmp(argv[1], "--test"))  return run_tests() ? 1 : 0;
    if (argc > 1 && !strcmp(argv[1], "--bench")) { bench(); return 0; }

    long long D, F; int n;
    if (scanf("%lld %lld %d", &D, &F, &n) != 3 || n < 0 || D < 0 || F < 0) { fprintf(stderr, "bad input\n"); return 1; }
    St *st = malloc((n + 1) * sizeof(St)); int *ch = malloc((n + 1) * sizeof(int));
    for (int i = 0; i < n; i++)
        if (scanf("%lld %lld", &st[i].d, &st[i].f) != 2 || st[i].d < 0 || st[i].f < 0) { fprintf(stderr, "bad station %d\n", i + 1); return 1; }
    qsort(st, n, sizeof(St), cmp_st);

    int r = min_stops(st, n, D, F, ch);
    if (r < 0) printf("Minimum stops: impossible (target cannot be reached with F = %lld)\n", F);
    else {
        printf("Minimum stops: %d\n", r);
        int *idx = ch;                                  /* print used stations in distance order */
        for (int i = 0; i < r; i++) for (int j = i + 1; j < r; j++) if (idx[j] < idx[i]) { int t = idx[i]; idx[i] = idx[j]; idx[j] = t; }
        for (int i = 0; i < r; i++) printf("  stop at distance %lld, refuel %lld\n", st[idx[i]].d, st[idx[i]].f);
    }
    printf("Minimum initial fuel for the target to be reachable at all: %lld\n", min_initial_fuel(st, n, D));
    free(st); free(ch);
    return 0;
}
