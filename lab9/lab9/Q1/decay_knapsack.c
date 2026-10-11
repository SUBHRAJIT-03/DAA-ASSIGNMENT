/*
 * Fractional Knapsack with Deterioration Rate
 * -------------------------------------------
 * Model
 *   item i : base value v_i, weight w_i, decay rate lambda_i > 0
 *            density d_i = v_i / w_i
 *   Items are consumed one after another at speed 1 weight-unit per time-unit,
 *   so "time" = total weight consumed so far.  The weight consumed at time t
 *   from item i yields value (d_i - lambda_i * t) per unit weight.
 *   Taking x_i of item i during [s_i, e_i] (e_i = s_i + x_i) gives
 *        gain_i = integral_{s_i}^{e_i} (d_i - lambda_i t) dt
 *   Constraints: 0 <= x_i <= w_i, sum x_i <= W.  Maximise sum gain_i.
 *
 * Algorithm (see README.md)
 *   1. Sort by lambda descending (exchange argument)       O(n log n)
 *   2. In end-time variables e_i the objective is separable concave with
 *      chain constraints e_{i-1} <= e_i <= e_{i-1}+w_i, e_n <= W.
 *      Solve it exactly by a slope-trick DP on piecewise-linear derivatives,
 *      then back-track.                                     O(n^2) (array impl.)
 *
 * Usage:  ./decay_knapsack < input.txt
 *         ./decay_knapsack --test      (validation against brute force + KKT)
 *         ./decay_knapsack --bench     (empirical running time)
 * Input:  n W
 *         v_1 w_1 lambda_1
 *         ...
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

typedef struct { int id; double v, w, lam, d; } Item;
typedef struct { double lo, hi, a, b; } Seg;   /* derivative = a + b*e on [lo,hi] */

static int cmp_item(const void *A, const void *B) {
    const Item *p = A, *q = B;
    if (p->lam > q->lam) return -1;
    if (p->lam < q->lam) return 1;
    if (p->d > q->d) return -1;
    if (p->d < q->d) return 1;
    return p->id - q->id;
}

static double gain(const Item *it, double x, double s) {
    return it->d * x - it->lam * (s * x + 0.5 * x * x);
}

/* items must already be sorted by lambda desc. Fills x[0..n-1]. */
static void solve(const Item *it, int n, double W, double *x) {
    Seg *cur = malloc((2 * n + 4) * sizeof(Seg));
    Seg *nxt = malloc((2 * n + 4) * sizeof(Seg));
    double *ystar = malloc(n * sizeof(double));
    int m = 0;
    double D = 0.0;                      /* current domain [0, D] */

    for (int i = 0; i < n; i++) {
        double w = it[i].w;
        /* locate peak y* of concave B_{i-1}: first place derivative <= 0 */
        double y = D;
        int kcut = m, partial = 0;
        for (int k = 0; k < m; k++) {
            double fl = cur[k].a + cur[k].b * cur[k].lo;
            double fh = cur[k].a + cur[k].b * cur[k].hi;
            if (fl <= 0) { y = cur[k].lo; kcut = k; break; }
            if (fh <= 0) { y = -cur[k].a / cur[k].b; kcut = k; partial = 1; break; }
        }
        ystar[i] = y;

        /* window-max: insert a flat piece of length w at y*, shift the right part */
        int c = 0;
        for (int k = 0; k < kcut; k++) nxt[c++] = cur[k];
        if (partial) {
            Seg s = cur[kcut]; s.hi = y;
            if (s.hi - s.lo > 1e-15) nxt[c++] = s;
        }
        Seg flat = { y, y + w, 0.0, 0.0 };
        nxt[c++] = flat;
        int rstart = kcut;
        if (partial) {
            Seg s = cur[kcut]; s.lo = y;
            s.a -= s.b * w; s.lo += w; s.hi += w;
            if (s.hi - s.lo > 1e-15) nxt[c++] = s;
            rstart = kcut + 1;
        }
        for (int k = rstart; k < m; k++) {
            Seg s = cur[k];
            s.a -= s.b * w; s.lo += w; s.hi += w;
            nxt[c++] = s;
        }
        /* add G_i'(e) = (d_i - d_{i+1}) - (lam_i - lam_{i+1}) e */
        double dn = (i + 1 < n) ? it[i + 1].d : 0.0;
        double ln = (i + 1 < n) ? it[i + 1].lam : 0.0;
        for (int k = 0; k < c; k++) {
            nxt[k].a += it[i].d - dn;
            nxt[k].b -= it[i].lam - ln;
        }
        Seg *t = cur; cur = nxt; nxt = t; m = c;
        D += w;
    }

    /* best final end time in [0, min(W, D)] */
    double limit = W < D ? W : D, ec = limit;
    for (int k = 0; k < m; k++) {
        double fl = cur[k].a + cur[k].b * cur[k].lo;
        double fh = cur[k].a + cur[k].b * cur[k].hi;
        if (fl <= 0) { ec = cur[k].lo; break; }
        if (fh <= 0) { ec = -cur[k].a / cur[k].b; break; }
    }
    if (ec > limit) ec = limit;
    if (ec < 0) ec = 0;

    /* back-track */
    double e = ec;
    for (int i = n - 1; i >= 0; i--) {
        double lo = e - it[i].w, prev = ystar[i];
        if (prev < lo) prev = lo;
        if (prev > e) prev = e;
        if (prev < 0) prev = 0;
        x[i] = e - prev;
        e = prev;
    }
    free(cur); free(nxt); free(ystar);
}

static double total_value(const Item *it, int n, const double *x) {
    double s = 0, tot = 0;
    for (int i = 0; i < n; i++) { tot += gain(&it[i], x[i], s); s += x[i]; }
    return tot;
}

/* exact optimality certificate (KKT) for the concave problem; returns 1 if ok */
static int kkt_ok(const Item *it, int n, double W, const double *x) {
    double *g = malloc(n * sizeof(double));
    double T = 0; for (int i = 0; i < n; i++) T += x[i];
    double L = 0, e = T;
    for (int i = n - 1; i >= 0; i--) {   /* g_i = d_i - lam_i e_i - sum_{j>i} lam_j x_j */
        g[i] = it[i].d - it[i].lam * e - L;
        L += it[i].lam * x[i];
        e -= x[i];
    }
    double eps = 1e-8, tol = 1e-6; int ok = 1;
    for (int i = 0; i < n && ok; i++) {
        if (x[i] < -eps || x[i] > it[i].w + eps) ok = 0;
        int up = x[i] < it[i].w - eps, dn = x[i] > eps;
        if (T < W - eps) {
            if (up && g[i] > tol) ok = 0;
            if (dn && g[i] < -tol) ok = 0;
        }
        for (int j = 0; j < n && ok; j++)
            if (i != j && up && x[j] > eps && g[i] - g[j] > tol) ok = 0;
    }
    if (T > W + 1e-7) ok = 0;
    free(g);
    return ok;
}

/* brute force for n<=3: all orders x grid of amounts */
static double brute(const Item *it, int n, double W, int G) {
    int perm[3], best_set = 0; double best = 0; (void)best_set;
    int idx[3] = {0, 1, 2};
    int perms[6][3] = {{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
    int np = (n == 1) ? 1 : (n == 2) ? 2 : 6;
    for (int p = 0; p < np; p++) {
        if (n == 1) { perm[0] = 0; }
        else if (n == 2) { perm[0] = p; perm[1] = 1 - p; }
        else { for (int k = 0; k < 3; k++) perm[k] = perms[p][k]; }
        (void)idx;
        int c[3] = {0, 0, 0}, lim[3] = {0, 0, 0};
        for (int k = 0; k < n; k++) lim[k] = G;
        for (c[0] = 0; c[0] <= lim[0]; c[0]++)
        for (c[1] = 0; c[1] <= (n > 1 ? lim[1] : 0); c[1]++)
        for (c[2] = 0; c[2] <= (n > 2 ? lim[2] : 0); c[2]++) {
            double s = 0, tot = 0; int ok = 1;
            for (int k = 0; k < n; k++) {
                const Item *q = &it[perm[k]];
                double xv = q->w * c[k] / G;
                s += 0; (void)s;
                tot += 0; (void)tot; (void)xv;
            }
            double S = 0, V = 0;
            for (int k = 0; k < n; k++) {
                const Item *q = &it[perm[k]];
                double xv = q->w * c[k] / G;
                V += gain(q, xv, S); S += xv;
            }
            if (S > W + 1e-12) ok = 0;
            if (ok && V > best) best = V;
        }
    }
    return best;
}

static double rnd(double a, double b) { return a + (b - a) * (rand() / (double)RAND_MAX); }

static void gen(Item *it, int n, double *W, int ties) {
    double sw = 0;
    for (int i = 0; i < n; i++) {
        it[i].id = i + 1;
        it[i].w = rnd(0.5, 10);
        it[i].d = rnd(1, 20);
        it[i].v = it[i].d * it[i].w;
        it[i].lam = ties ? (double)(1 + rand() % 3) * 0.3 : rnd(0.01, 2.0);
        sw += it[i].w;
    }
    *W = sw * rnd(0.1, 1.3);
}

static int run_tests(void) {
    srand(12345);
    int fail = 0, N1 = 300, N2 = 2000;
    double worst_gap = 0;
    for (int t = 0; t < N1; t++) {                 /* brute-force comparison */
        int n = 1 + rand() % 3; Item it[3]; double W, x[3];
        gen(it, n, &W, t % 4 == 0);
        qsort(it, n, sizeof(Item), cmp_item);
        solve(it, n, W, x);
        double a = total_value(it, n, x), b = brute(it, n, W, 40);
        if (a < b - 1e-9) { fail++; printf("BRUTE FAIL t=%d algo=%.6f brute=%.6f\n", t, a, b); }
        if (b > 0 && (a - b) / b > worst_gap) worst_gap = (a - b) / b;
    }
    printf("brute-force check (n<=3, all orders, grid): %d/%d passed (grid is coarser, algo >= grid)\n", N1 - fail, N1);
    int f2 = 0;
    for (int t = 0; t < N2; t++) {                 /* KKT certificate */
        int n = 1 + rand() % 60; Item *it = malloc(n * sizeof(Item)); double W; double *x = malloc(n * sizeof(double));
        gen(it, n, &W, t % 5 == 0);
        qsort(it, n, sizeof(Item), cmp_item);
        solve(it, n, W, x);
        if (!kkt_ok(it, n, W, x)) { f2++; printf("KKT FAIL t=%d n=%d\n", t, n); }
        free(it); free(x);
    }
    printf("KKT optimality certificate (n<=60):          %d/%d passed\n", N2 - f2, N2);
    return fail + f2;
}

static void bench(void) {
    srand(7);
    for (int n = 2000; n <= 32000; n *= 2) {
        Item *it = malloc(n * sizeof(Item)); double *x = malloc(n * sizeof(double)); double W;
        gen(it, n, &W, 0);
        clock_t c0 = clock();
        qsort(it, n, sizeof(Item), cmp_item);
        solve(it, n, W, x);
        printf("n=%6d  time=%.4f s\n", n, (double)(clock() - c0) / CLOCKS_PER_SEC);
        free(it); free(x);
    }
}

int main(int argc, char **argv) {
    if (argc > 1 && !strcmp(argv[1], "--test"))  return run_tests() ? 1 : 0;
    if (argc > 1 && !strcmp(argv[1], "--bench")) { bench(); return 0; }

    int n; double W;
    if (scanf("%d %lf", &n, &W) != 2 || n <= 0 || W < 0) { fprintf(stderr, "bad input\n"); return 1; }
    Item *it = malloc(n * sizeof(Item)); double *x = malloc(n * sizeof(double));
    for (int i = 0; i < n; i++) {
        if (scanf("%lf %lf %lf", &it[i].v, &it[i].w, &it[i].lam) != 3 ||
            it[i].w <= 0 || it[i].lam <= 0) { fprintf(stderr, "bad item %d\n", i + 1); return 1; }
        it[i].id = i + 1; it[i].d = it[i].v / it[i].w;
    }
    qsort(it, n, sizeof(Item), cmp_item);
    solve(it, n, W, x);

    printf("Optimal schedule (consume in this order):\n");
    printf("%-5s %-6s %-9s %-8s %-9s %-9s %-9s %-9s %-10s\n",
           "ord", "item", "density", "lambda", "weight", "fraction", "start", "end", "gain");
    double s = 0, tot = 0;
    for (int i = 0; i < n; i++) {
        double g = gain(&it[i], x[i], s);
        printf("%-5d %-6d %-9.4f %-8.4f %-9.4f %-9.4f %-9.4f %-9.4f %-10.4f\n",
               i + 1, it[i].id, it[i].d, it[i].lam, x[i], x[i] / it[i].w, s, s + x[i], g);
        s += x[i]; tot += g;
    }
    printf("Total weight used = %.4f / %.4f\n", s, W);
    printf("Maximum total value = %.6f\n", tot);
    free(it); free(x);
    return 0;
}
