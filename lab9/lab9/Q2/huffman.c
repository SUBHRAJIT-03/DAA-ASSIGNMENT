/*
 * Huffman Coding with canonical codebook
 * --------------------------------------
 * Input representation
 *   Each symbol is a pair (name, frequency) stored in a flat array.
 *   Symbols are sorted once by frequency; the tree is then built with the
 *   TWO-QUEUE method (a sorted queue of leaves + a queue of internal nodes
 *   that is automatically sorted), so no heap is needed:
 *        sort: O(n log n),  tree construction: O(n).
 *   The tree is stored implicitly as parent[] over 2m-1 nodes; only the
 *   code LENGTHS are needed, because the canonical codebook is determined
 *   by lengths alone.
 *
 * Canonical codes: symbols ordered by (length, name); the first code is
 *   0...0, each next code = (previous + 1) shifted left by the length gap.
 *
 * Usage:  ./huffman < input.txt
 *         ./huffman --test      (validation: brute force, naive baseline, prefix/Kraft)
 *         ./huffman --bench     (empirical timing)
 * Input:  n
 *         name_1 freq_1
 *         ...
 * Symbols with frequency 0 get no code.  A lone symbol gets code "0".
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

typedef struct { char name[32]; long long f; int len; char *code; } Sym;
typedef struct { long long f; int idx; } FI;

static int cmp_fi(const void *A, const void *B) {
    const FI *p = A, *q = B;
    if (p->f != q->f) return p->f < q->f ? -1 : 1;
    return p->idx - q->idx;
}

/* Computes code lengths len[0..n-1] (0 for zero-frequency symbols). */
static void huffman_lengths(const long long *freq, int n, int *len) {
    for (int i = 0; i < n; i++) len[i] = 0;
    FI *a = malloc((n + 1) * sizeof(FI));
    int m = 0;
    for (int i = 0; i < n; i++) if (freq[i] > 0) { a[m].f = freq[i]; a[m].idx = i; m++; }
    if (m == 0) { free(a); return; }
    if (m == 1) { len[a[0].idx] = 1; free(a); return; }
    qsort(a, m, sizeof(FI), cmp_fi);

    int total = 2 * m - 1;
    long long *w = malloc(total * sizeof(long long));
    int *par = malloc(total * sizeof(int));
    int *dep = malloc(total * sizeof(int));
    for (int i = 0; i < m; i++) w[i] = a[i].f;

    int i = 0, j = m, k = m;      /* i: next leaf, j: next internal, k: next to create */
    for (; k < total; k++) {
        int c[2];
        for (int t = 0; t < 2; t++) {
            if (i < m && (j >= k || w[i] <= w[j])) c[t] = i++;   /* ties: prefer leaf */
            else c[t] = j++;
        }
        w[k] = w[c[0]] + w[c[1]];
        par[c[0]] = par[c[1]] = k;
    }
    dep[total - 1] = 0;
    for (int v = total - 2; v >= 0; v--) dep[v] = dep[par[v]] + 1;  /* parent index > child index */
    for (int v = 0; v < m; v++) len[a[v].idx] = dep[v];
    free(a); free(w); free(par); free(dep);
}

static Sym *g_s;
static int cmp_canon(const void *A, const void *B) {
    const Sym *p = &g_s[*(const int *)A], *q = &g_s[*(const int *)B];
    if (p->len != q->len) return p->len - q->len;
    return strcmp(p->name, q->name);
}

/* Fills s[i].code (malloc'd strings); returns canonical order in ord[0..cnt-1], cnt returned. */
static int canonical_codes(Sym *s, int n, int *ord) {
    int cnt = 0, maxlen = 0;
    for (int i = 0; i < n; i++) if (s[i].len > 0) { ord[cnt++] = i; if (s[i].len > maxlen) maxlen = s[i].len; }
    g_s = s;
    qsort(ord, cnt, sizeof(int), cmp_canon);
    char *buf = malloc(maxlen + 2);
    int cur = 0;
    for (int k = 0; k < cnt; k++) {
        Sym *q = &s[ord[k]];
        if (k == 0) { cur = q->len; memset(buf, '0', cur); }
        else {
            for (int p = cur - 1; p >= 0; p--) {           /* binary +1 */
                if (buf[p] == '0') { buf[p] = '1'; break; }
                buf[p] = '0';
            }
            for (int p = cur; p < q->len; p++) buf[p] = '0';  /* shift left by gap */
            cur = q->len;
        }
        q->code = malloc(cur + 1);
        memcpy(q->code, buf, cur); q->code[cur] = 0;
    }
    free(buf);
    return cnt;
}

static long long total_cost(const Sym *s, int n) {
    long long c = 0;
    for (int i = 0; i < n; i++) c += s[i].f * s[i].len;
    return c;
}

/* ---------------- validation helpers ---------------- */
static long long naive_cost(const long long *f, int n) {      /* O(n^2) textbook Huffman, cost = sum of merges */
    long long *w = malloc(n * sizeof(long long)); int m = 0;
    for (int i = 0; i < n; i++) if (f[i] > 0) w[m++] = f[i];
    long long cost = 0;
    if (m == 1) { free(w); return f[0] > 0 ? f[0] : 0; }
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

static long long best;
static void dfs(int i, int n, const long long *f, long long ks, long long full, long long cost, int maxl) {
    if (i == n) { if (ks == full && cost < best) best = cost; return; }
    for (int l = 1; l <= maxl; l++) {
        long long t = 1LL << (maxl - l);
        if (ks + t > full) continue;
        dfs(i + 1, n, f, ks + t, full, cost + f[i] * l, maxl);
    }
}
/* minimum over ALL length vectors satisfying Kraft with equality (= all full binary trees) */
static long long brute_cost(const long long *f, int n) {
    best = (long long)4e18;
    int maxl = n - 1;
    dfs(0, n, f, 0, 1LL << maxl, 0, maxl);
    return best;
}

static int prefix_free(const Sym *s, int n) {
    for (int i = 0; i < n; i++) if (s[i].len > 0)
        for (int j = 0; j < n; j++) if (i != j && s[j].len >= s[i].len && s[j].len > 0 &&
            strncmp(s[i].code, s[j].code, s[i].len) == 0) return 0;
    return 1;
}
static int kraft_ok(const Sym *s, int n, int cnt) {
    if (cnt == 1) return 1;
    long double k = 0;
    for (int i = 0; i < n; i++) if (s[i].len > 0) k += ldexpl(1.0L, -s[i].len);
    return fabsl(k - 1.0L) < 1e-12L;
}
static int canonical_ok(const Sym *s, const int *ord, int cnt) {
    for (int k = 1; k < cnt; k++) {
        const Sym *p = &s[ord[k - 1]], *q = &s[ord[k]];
        if (q->len < p->len) return 0;
        if (q->len > 60) return 1;                       /* numeric check only for short codes */
        unsigned long long a = strtoull(p->code, NULL, 2), b = strtoull(q->code, NULL, 2);
        if (b != ((a + 1) << (q->len - p->len))) return 0;
    }
    return 1;
}

static void build(Sym *s, const long long *f, int n) {
    int *len = malloc(n * sizeof(int));
    huffman_lengths(f, n, len);
    for (int i = 0; i < n; i++) { snprintf(s[i].name, 32, "s%05d", i); s[i].f = f[i]; s[i].len = len[i]; s[i].code = NULL; }
    free(len);
}
static void free_syms(Sym *s, int n) { for (int i = 0; i < n; i++) free(s[i].code); }

static int check_all(long long *f, int n, int small, int *fail_reason) {
    Sym *s = malloc(n * sizeof(Sym)); int *ord = malloc(n * sizeof(int));
    build(s, f, n);
    int cnt = canonical_codes(s, n, ord);
    long long c = total_cost(s, n), ref = naive_cost(f, n);
    int ok = 1; *fail_reason = 0;
    if (c != ref) { ok = 0; *fail_reason = 1; }
    if (small && n >= 2 && c != brute_cost(f, n)) { ok = 0; *fail_reason = 2; }
    if (!prefix_free(s, n)) { ok = 0; *fail_reason = 3; }
    if (!kraft_ok(s, n, cnt)) { ok = 0; *fail_reason = 4; }
    if (!canonical_ok(s, ord, cnt)) { ok = 0; *fail_reason = 5; }
    free_syms(s, n); free(s); free(ord);
    return ok;
}

static int run_tests(void) {
    srand(2024);
    int bad = 0, T1 = 400;
    for (int t = 0; t < T1; t++) {                       /* brute force, n <= 8, many ties */
        int n = 2 + rand() % 7; long long f[8]; int r;
        for (int i = 0; i < n; i++) f[i] = 1 + rand() % (t % 2 ? 6 : 50);
        if (!check_all(f, n, 1, &r)) { bad++; printf("FAIL small t=%d reason=%d\n", t, r); }
    }
    printf("brute force (all full trees, n<=8) + checks : %d/%d passed\n", T1 - bad, T1);
    int bad2 = 0, T2 = 300;
    for (int t = 0; t < T2; t++) {                       /* naive baseline, n <= 200 */
        int n = 1 + rand() % 200; long long *f = malloc(n * sizeof(long long)); int r;
        for (int i = 0; i < n; i++) f[i] = (rand() % 15 == 0) ? 0 : 1 + rand() % (t % 3 ? 1000 : 4);
        if (!check_all(f, n, 0, &r)) { bad2++; printf("FAIL large t=%d reason=%d\n", t, r); }
        free(f);
    }
    printf("naive O(n^2) baseline + prefix/Kraft/canonical : %d/%d passed\n", T2 - bad2, T2);
    long long fib[80]; fib[0] = 1; fib[1] = 1; int r;
    for (int i = 2; i < 80; i++) fib[i] = fib[i - 1] + fib[i - 2];
    int okf = check_all(fib, 80, 0, &r);                 /* worst-case depth 79 */
    printf("Fibonacci frequencies (depth n-1)               : %s\n", okf ? "passed" : "FAILED");
    return bad + bad2 + !okf;
}

static void bench(void) {
    srand(5);
    for (int n = 100000; n <= 1600000; n *= 2) {
        long long *f = malloc(n * sizeof(long long)); int *len = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) f[i] = 1 + rand() % 1000000;
        clock_t c0 = clock();
        huffman_lengths(f, n, len);
        printf("n=%8d  time=%.4f s\n", n, (double)(clock() - c0) / CLOCKS_PER_SEC);
        free(f); free(len);
    }
}

int main(int argc, char **argv) {
    if (argc > 1 && !strcmp(argv[1], "--test"))  return run_tests() ? 1 : 0;
    if (argc > 1 && !strcmp(argv[1], "--bench")) { bench(); return 0; }

    int n;
    if (scanf("%d", &n) != 1 || n <= 0) { fprintf(stderr, "bad input\n"); return 1; }
    Sym *s = calloc(n, sizeof(Sym)); long long *f = malloc(n * sizeof(long long));
    for (int i = 0; i < n; i++) {
        if (scanf("%31s %lld", s[i].name, &s[i].f) != 2 || s[i].f < 0) { fprintf(stderr, "bad symbol %d\n", i + 1); return 1; }
        f[i] = s[i].f;
    }
    int *len = malloc(n * sizeof(int)), *ord = malloc(n * sizeof(int));
    huffman_lengths(f, n, len);
    for (int i = 0; i < n; i++) s[i].len = len[i];
    int cnt = canonical_codes(s, n, ord);

    printf("Canonical Huffman codebook:\n%-12s %-10s %-6s %s\n", "symbol", "freq", "len", "code");
    for (int k = 0; k < cnt; k++) {
        Sym *q = &s[ord[k]];
        printf("%-12s %-10lld %-6d %s\n", q->name, q->f, q->len, q->code);
    }
    long long tot = 0, bits = total_cost(s, n); double H = 0;
    for (int i = 0; i < n; i++) tot += s[i].f;
    for (int i = 0; i < n; i++) if (s[i].f > 0) { double p = (double)s[i].f / tot; H -= p * log2(p); }
    printf("Total encoded bits = %lld\n", bits);
    if (tot > 0) printf("Expected length = %.6f bits/symbol  (entropy = %.6f)\n", (double)bits / tot, H);
    free_syms(s, n); free(s); free(f); free(len); free(ord);
    return 0;
}
