/*
 * Reorganise String with K-Distance Apart
 * ---------------------------------------
 * Condition: two equal characters at positions i < j must satisfy j - i >= K.
 *            (K <= 1 puts no restriction.)
 *
 * Input representation
 *   The string is reduced to a frequency table cnt[256] over the d distinct
 *   characters, plus a "cooldown" table ready[c] = earliest position where
 *   character c may be placed again.  Position by position we pick, among
 *   the characters that are ready and still have copies left, the one with
 *   the LARGEST remaining count (ties: smaller character).
 *     scan version : each step scans the d distinct characters -> O(n*d), d <= 256
 *     heap version : max-heap of ready characters + FIFO cooldown queue -> O(n log d)
 *   Both choose exactly the same character at every step.
 *
 * Usage:  ./reorganize < input.txt    ./reorganize --test    ./reorganize --bench
 * Input:  K
 *         S            (the rest of the second line, any bytes except newline)
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---------- version 1: scan of the distinct characters ---------- */
static int reorg_scan(const unsigned char *s, int n, int K, unsigned char *out) {
    int cnt[256] = {0}, ready[256] = {0}, ch[256], d = 0;
    for (int i = 0; i < n; i++) cnt[s[i]]++;
    for (int c = 0; c < 256; c++) if (cnt[c]) ch[d++] = c;
    int gap = K > 1 ? K : 1;
    for (int pos = 0; pos < n; pos++) {
        int best = -1;
        for (int t = 0; t < d; t++) {
            int c = ch[t];
            if (cnt[c] > 0 && ready[c] <= pos && (best < 0 || cnt[c] > cnt[best])) best = c;
        }
        if (best < 0) return 0;
        out[pos] = (unsigned char)best;
        cnt[best]--; ready[best] = pos + gap;
    }
    return 1;
}

/* ---------- version 2: max-heap + cooldown queue ---------- */
typedef struct { int cnt, ch; } HE;
static int better(HE a, HE b) { return a.cnt > b.cnt || (a.cnt == b.cnt && a.ch < b.ch); }
static void hpush(HE *h, int *sz, HE x) {
    int i = (*sz)++;
    while (i > 0 && better(x, h[(i - 1) / 2])) { h[i] = h[(i - 1) / 2]; i = (i - 1) / 2; }
    h[i] = x;
}
static HE hpop(HE *h, int *sz) {
    HE top = h[0], x = h[--(*sz)]; int i = 0;
    for (;;) {
        int c = 2 * i + 1;
        if (c >= *sz) break;
        if (c + 1 < *sz && better(h[c + 1], h[c])) c++;
        if (!better(h[c], x)) break;
        h[i] = h[c]; i = c;
    }
    if (*sz > 0) h[i] = x;
    return top;
}
static int reorg_heap(const unsigned char *s, int n, int K, unsigned char *out) {
    int cnt[256] = {0};
    for (int i = 0; i < n; i++) cnt[s[i]]++;
    HE heap[300]; int sz = 0;
    for (int c = 0; c < 256; c++) if (cnt[c]) { HE e = { cnt[c], c }; hpush(heap, &sz, e); }
    HE q[512]; int qpos[512], qh = 0, qt = 0;               /* FIFO cooldown queue (release times increase) */
    int gap = K > 1 ? K : 1;
    for (int pos = 0; pos < n; pos++) {
        while (qh != qt && qpos[qh] <= pos) { hpush(heap, &sz, q[qh]); qh = (qh + 1) & 511; }
        if (sz == 0) return 0;
        HE b = hpop(heap, &sz);
        out[pos] = (unsigned char)b.ch;
        b.cnt--;
        if (b.cnt > 0) { q[qt] = b; qpos[qt] = pos + gap; qt = (qt + 1) & 511; }
    }
    return 1;
}

/* necessary and (as tested) sufficient condition, O(d): (m-1)*K + c <= n */
static int feasible_formula(const unsigned char *s, int n, int K) {
    if (K <= 1 || n == 0) return 1;
    int cnt[256] = {0}, m = 0, c = 0;
    for (int i = 0; i < n; i++) cnt[s[i]]++;
    for (int x = 0; x < 256; x++) { if (cnt[x] > m) { m = cnt[x]; c = 1; } else if (cnt[x] == m && m > 0) c++; }
    return (long long)(m - 1) * K + c <= n;
}

/* ---------------- validation helpers ---------------- */
static int valid(const unsigned char *s, int n, const unsigned char *out, int K) {
    int a[256] = {0}, b[256] = {0}, last[256];
    for (int i = 0; i < n; i++) { a[s[i]]++; b[out[i]]++; last[i % 256] = 0; }
    for (int c = 0; c < 256; c++) { if (a[c] != b[c]) return 0; last[c] = -1000000000; }
    for (int i = 0; i < n; i++) {
        if (K > 1 && i - last[out[i]] < K) return 0;
        last[out[i]] = i;
    }
    return 1;
}
static int bf_cnt[256], bf_last[256], bf_chars[16], bf_d;
static int bf_dfs(int pos, int n, int K) {                 /* does ANY valid arrangement exist? */
    if (pos == n) return 1;
    for (int t = 0; t < bf_d; t++) {
        int c = bf_chars[t];
        if (bf_cnt[c] == 0) continue;
        if (bf_last[c] >= 0 && pos - bf_last[c] < K) continue;
        int old = bf_last[c];
        bf_cnt[c]--; bf_last[c] = pos;
        int ok = bf_dfs(pos + 1, n, K);
        bf_cnt[c]++; bf_last[c] = old;
        if (ok) return 1;
    }
    return 0;
}
static int brute_feasible(const unsigned char *s, int n, int K) {
    memset(bf_cnt, 0, sizeof bf_cnt); bf_d = 0;
    for (int i = 0; i < n; i++) { if (bf_cnt[s[i]] == 0) bf_chars[bf_d++] = s[i]; bf_cnt[s[i]]++; }
    for (int c = 0; c < 256; c++) bf_last[c] = -1;
    return bf_dfs(0, n, K);
}

static int run_tests(void) {
    srand(4242);
    int bad = 0, T1 = 3000;
    for (int t = 0; t < T1; t++) {                      /* brute force over all arrangements */
        int n = rand() % 10, sig = 1 + rand() % 4, K = rand() % 7;
        unsigned char s[16], o1[16], o2[16];
        for (int i = 0; i < n; i++) s[i] = 'a' + rand() % sig;
        int bf = brute_feasible(s, n, K);
        int r1 = reorg_scan(s, n, K, o1), r2 = reorg_heap(s, n, K, o2), fm = feasible_formula(s, n, K);
        int ok = (r1 == bf) && (r2 == bf) && (fm == bf);
        if (r1 && !valid(s, n, o1, K)) ok = 0;
        if (r2 && !valid(s, n, o2, K)) ok = 0;
        if (r1 && r2 && memcmp(o1, o2, n) != 0) ok = 0;
        if (!ok) { bad++; printf("FAIL brute t=%d n=%d K=%d bf=%d scan=%d heap=%d formula=%d\n", t, n, K, bf, r1, r2, fm); }
    }
    printf("brute force (n<=9, all arrangements, K=0..6)      : %d/%d passed\n", T1 - bad, T1);
    int bad2 = 0, T2 = 1000, feas = 0;
    for (int t = 0; t < T2; t++) {                      /* larger random inputs */
        int n = 1 + rand() % 2000, sig = 1 + rand() % (t % 3 == 0 ? 256 : 12), K = rand() % 20;
        unsigned char *s = malloc(n + 1), *o1 = malloc(n + 1), *o2 = malloc(n + 1);
        for (int i = 0; i < n; i++) s[i] = rand() % sig;
        int r1 = reorg_scan(s, n, K, o1), r2 = reorg_heap(s, n, K, o2), fm = feasible_formula(s, n, K);
        int ok = (r1 == r2) && (r1 == fm);
        if (r1 && (!valid(s, n, o1, K) || memcmp(o1, o2, n) != 0)) ok = 0;
        feas += r1;
        if (!ok) { bad2++; printf("FAIL large t=%d n=%d K=%d scan=%d heap=%d formula=%d\n", t, n, K, r1, r2, fm); }
        free(s); free(o1); free(o2);
    }
    printf("scan = heap = formula, outputs valid (n<=2000)    : %d/%d passed (%d feasible, %d impossible)\n", T2 - bad2, T2, feas, T2 - feas);
    unsigned char o[32]; int ok = 1;
    ok &= reorg_scan((const unsigned char *)"aabbcc", 6, 3, o) && memcmp(o, "abcabc", 6) == 0;
    ok &= !reorg_scan((const unsigned char *)"aabbcc", 6, 4, o);
    ok &= reorg_scan((const unsigned char *)"aaadbbcc", 8, 2, o) && memcmp(o, "abacabcd", 8) == 0;
    ok &= !reorg_scan((const unsigned char *)"aaabc", 5, 3, o);
    ok &= reorg_scan((const unsigned char *)"aaab", 4, 1, o) && reorg_scan((const unsigned char *)"aaab", 4, 0, o);
    ok &= reorg_scan((const unsigned char *)"", 0, 5, o);
    ok &= reorg_scan((const unsigned char *)"a", 1, 9, o) && !reorg_scan((const unsigned char *)"aa", 2, 2, o);
    printf("special cases (known answers, K<=1, empty, single) : %s\n", ok ? "passed" : "FAILED");
    return bad + bad2 + !ok;
}

static double now(void) { return (double)clock() / CLOCKS_PER_SEC; }
static void bench(void) {
    srand(5);
    printf("-- growth in n (26 letters, K=5) --\n");
    for (int n = 200000; n <= 6400000; n *= 2) {
        unsigned char *s = malloc(n), *o = malloc(n);
        for (int i = 0; i < n; i++) s[i] = 'a' + rand() % 26;
        double t0 = now(); int a = reorg_scan(s, n, 5, o); double t1 = now() - t0;
        t0 = now(); int b = reorg_heap(s, n, 5, o); double t2 = now() - t0;
        printf("n=%8d  scan=%.4f s  heap=%.4f s  (feasible %d/%d)\n", n, t1, t2, a, b);
        free(s); free(o);
    }
    printf("-- growth in alphabet size d (n = 1 000 000, K=3) --\n");
    int n = 1000000, ds[] = {2, 4, 16, 64, 256};
    for (int u = 0; u < 5; u++) {
        int d = ds[u];
        unsigned char *s = malloc(n), *o = malloc(n);
        for (int i = 0; i < n; i++) s[i] = rand() % d;
        double t0 = now(); int a = reorg_scan(s, n, d == 2 ? 2 : 3, o); double t1 = now() - t0;
        t0 = now(); int b = reorg_heap(s, n, d == 2 ? 2 : 3, o); double t2 = now() - t0;
        printf("d=%3d  scan=%.4f s  heap=%.4f s  (feasible %d/%d)\n", d, t1, t2, a, b);
        free(s); free(o);
    }
}

int main(int argc, char **argv) {
    if (argc > 1 && !strcmp(argv[1], "--test"))  return run_tests() ? 1 : 0;
    if (argc > 1 && !strcmp(argv[1], "--bench")) { bench(); return 0; }

    int K;
    if (scanf("%d", &K) != 1) { fprintf(stderr, "bad input\n"); return 1; }
    int c; while ((c = getchar()) != '\n' && c != EOF) ;       /* rest of the K line */
    char *line = NULL; size_t cap = 0;
    ssize_t len = getline(&line, &cap, stdin);
    if (len < 0) len = 0;
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) len--;
    unsigned char *out = malloc(len + 1);
    int ok = reorg_scan((unsigned char *)line, (int)len, K, out);
    printf("K = %d, n = %zd\n", K, len);
    if (ok) { printf("Rearranged string: \""); fwrite(out, 1, len, stdout); printf("\"\n"); }
    else printf("Rearranged string: \"\"  (impossible)\n");
    free(line); free(out);
    return 0;
}
