/*
 * Minimum Number of Meeting Rooms
 * -------------------------------
 * Meetings are half-open intervals [s, e) with s < e, so a meeting that ends
 * at time t and one that starts at t may share a room.
 *
 * Input representation
 *   The pairing of start and end is irrelevant for COUNTING rooms: only the
 *   number of meetings "in progress" at each moment matters.  So the n
 *   intervals are split into two plain arrays, starts S[] and ends E[],
 *   each sorted independently, and swept like a merge:
 *        S[i] <  E[j]  -> a meeting begins before the next one ends: +1
 *        S[i] >= E[j]  -> a meeting ends first (ties: end first):    -1
 *   The maximum of the running count is the answer.      O(n log n), O(n) space.
 *
 *   To also PRODUCE a schedule, the meetings are sorted by start and kept in
 *   a min-heap of busy rooms keyed by end time, plus a min-heap of free room ids.
 *
 * Usage:  ./meeting_rooms < input.txt    ./meeting_rooms --test    ./meeting_rooms --bench
 * Input:  n
 *         s_1 e_1
 *         ...
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct { long long s, e; int id; } Mt;
typedef struct { long long key; int val; } HE;           /* min-heap */

static int cmp_ll(const void *A, const void *B) {
    long long a = *(const long long *)A, b = *(const long long *)B;
    return (a > b) - (a < b);
}
static int cmp_mt(const void *A, const void *B) {
    const Mt *p = A, *q = B;
    if (p->s != q->s) return p->s < q->s ? -1 : 1;
    if (p->e != q->e) return p->e < q->e ? -1 : 1;
    return p->id - q->id;
}

static void hpush(HE *h, int *sz, HE x) {
    int i = (*sz)++;
    while (i > 0 && h[(i - 1) / 2].key > x.key) { h[i] = h[(i - 1) / 2]; i = (i - 1) / 2; }
    h[i] = x;
}
static HE hpop(HE *h, int *sz) {
    HE top = h[0], x = h[--(*sz)]; int i = 0;
    for (;;) {
        int c = 2 * i + 1;
        if (c >= *sz) break;
        if (c + 1 < *sz && h[c + 1].key < h[c].key) c++;
        if (h[c].key >= x.key) break;
        h[i] = h[c]; i = c;
    }
    if (*sz > 0) h[i] = x;
    return top;
}

/* ---- main algorithm: two sorted endpoint arrays ---- */
static int rooms_sweep(const Mt *m, int n) {
    if (n == 0) return 0;
    long long *S = malloc(n * sizeof(long long)), *E = malloc(n * sizeof(long long));
    for (int i = 0; i < n; i++) { S[i] = m[i].s; E[i] = m[i].e; }
    qsort(S, n, sizeof(long long), cmp_ll);
    qsort(E, n, sizeof(long long), cmp_ll);
    int i = 0, j = 0, cur = 0, best = 0;
    while (i < n) {
        if (S[i] < E[j]) { cur++; i++; if (cur > best) best = cur; }
        else { cur--; j++; }
    }
    free(S); free(E);
    return best;
}

/* ---- schedule: room[id] for every meeting; returns number of rooms ---- */
static int assign_rooms(const Mt *m, int n, int *room) {
    if (n == 0) return 0;
    Mt *a = malloc(n * sizeof(Mt)); memcpy(a, m, n * sizeof(Mt));
    qsort(a, n, sizeof(Mt), cmp_mt);
    HE *busy = malloc((n + 1) * sizeof(HE)), *fre = malloc((n + 1) * sizeof(HE));
    int nb = 0, nf = 0, rooms = 0;
    for (int k = 0; k < n; k++) {
        while (nb > 0 && busy[0].key <= a[k].s) { HE t = hpop(busy, &nb); HE f = { t.val, 0 }; hpush(fre, &nf, f); }
        int r;
        if (nf > 0) r = (int)hpop(fre, &nf).key; else r = rooms++;
        HE b = { a[k].e, r }; hpush(busy, &nb, b);
        room[a[k].id] = r;
    }
    free(a); free(busy); free(fre);
    return rooms;
}

/* ---------------- baselines for validation ---------------- */
static int rooms_naive(const Mt *m, int n) {              /* O(n^2): max overlap at some start point */
    int best = 0;
    for (int i = 0; i < n; i++) {
        int c = 0;
        for (int j = 0; j < n; j++) if (m[j].s <= m[i].s && m[i].s < m[j].e) c++;
        if (c > best) best = c;
    }
    return best;
}
static int rooms_diff(const Mt *m, int n, int T) {        /* difference array, times in [0,T) */
    int *d = calloc(T + 2, sizeof(int)), cur = 0, best = 0;
    for (int i = 0; i < n; i++) { d[m[i].s]++; d[m[i].e]--; }
    for (int t = 0; t <= T; t++) { cur += d[t]; if (cur > best) best = cur; }
    free(d);
    return best;
}
static int bc_room[16];
static int overlap(const Mt *a, const Mt *b) { return a->s < b->e && b->s < a->e; }
static int bc_dfs(const Mt *m, int n, int i, int k, int used) {   /* can we colour with k rooms? */
    if (i == n) return 1;
    for (int r = 0; r < k && r <= used; r++) {
        int ok = 1;
        for (int j = 0; j < i && ok; j++) if (bc_room[j] == r && overlap(&m[i], &m[j])) ok = 0;
        if (!ok) continue;
        bc_room[i] = r;
        if (bc_dfs(m, n, i + 1, k, r == used ? used + 1 : used)) return 1;
    }
    return 0;
}
static int rooms_brute(const Mt *m, int n) {              /* smallest k admitting a conflict-free assignment */
    for (int k = 1; k <= n; k++) if (bc_dfs(m, n, 0, k, 0)) return k;
    return 0;
}
static int schedule_valid(const Mt *m, int n, const int *room, int rooms) {
    for (int i = 0; i < n; i++) if (room[i] < 0 || room[i] >= rooms) return 0;
    for (int i = 0; i < n; i++) for (int j = i + 1; j < n; j++)
        if (room[i] == room[j] && overlap(&m[i], &m[j])) return 0;
    return 1;
}

static void gen(Mt *m, int n, int T, int maxlen) {
    for (int i = 0; i < n; i++) {
        m[i].s = rand() % T; m[i].e = m[i].s + 1 + rand() % maxlen; m[i].id = i;
    }
}

static int run_tests(void) {
    srand(1234);
    int bad = 0, T1 = 3000;
    for (int t = 0; t < T1; t++) {                      /* exhaustive room assignment, n <= 7 */
        int n = rand() % 8; Mt m[8]; int room[8];
        gen(m, n, 10, t % 2 ? 4 : 10);
        int a = rooms_sweep(m, n), b = assign_rooms(m, n, room), c = rooms_naive(m, n), d = rooms_brute(m, n);
        if (a != d || b != d || c != d || !schedule_valid(m, n, room, b)) { bad++; printf("FAIL brute t=%d sweep=%d heap=%d naive=%d brute=%d\n", t, a, b, c, d); }
    }
    printf("exhaustive colouring (n<=7, many touching/ties)    : %d/%d passed\n", T1 - bad, T1);
    int bad2 = 0, T2 = 600;
    for (int t = 0; t < T2; t++) {                      /* larger inputs */
        int n = 1 + rand() % 1500, T = t % 3 == 0 ? 50 : t % 3 == 1 ? 1000 : 100000, L = t % 5 == 0 ? 3 : 200;
        Mt *m = malloc(n * sizeof(Mt)); int *room = malloc(n * sizeof(int));
        gen(m, n, T, L);
        if (t % 7 == 0) for (int i = 0; i < n; i++) { m[i].s = 5; m[i].e = 9; }     /* identical */
        int a = rooms_sweep(m, n), b = assign_rooms(m, n, room), c = rooms_naive(m, n);
        int ok = (a == b) && (a == c) && schedule_valid(m, n, room, b);
        if (T <= 100000) {                               /* difference array needs times in range */
            long long mx = 0; for (int i = 0; i < n; i++) if (m[i].e > mx) mx = m[i].e;
            ok = ok && rooms_diff(m, n, (int)mx) == a;
        }
        if (!ok) { bad2++; printf("FAIL large t=%d n=%d sweep=%d heap=%d naive=%d\n", t, n, a, b, c); }
        free(m); free(room);
    }
    printf("sweep = heap = naive = difference array (n<=1500)   : %d/%d passed\n", T2 - bad2, T2);
    int ok = 1;
    Mt x1[] = {{0,30,0},{5,10,1},{15,20,2}}, x2[] = {{7,10,0},{2,4,1}}, x3[] = {{1,2,0},{2,3,1}},
       x4[] = {{0,10,0},{1,9,1},{2,8,2},{3,7,3}}, x5[] = {{0,5,0},{5,10,1},{10,15,2},{0,15,3}};
    ok &= rooms_sweep(x1, 3) == 2 && rooms_sweep(x2, 2) == 1 && rooms_sweep(x3, 2) == 1;
    ok &= rooms_sweep(x4, 4) == 4 && rooms_sweep(x5, 4) == 2 && rooms_sweep(NULL, 0) == 0;
    Mt *id = malloc(100 * sizeof(Mt)); for (int i = 0; i < 100; i++) { id[i].s = 3; id[i].e = 4; id[i].id = i; }
    ok &= rooms_sweep(id, 100) == 100; free(id);
    printf("special cases (touching, nested, identical, n=0)     : %s\n", ok ? "passed" : "FAILED");
    return bad + bad2 + !ok;
}

static double now(void) { return (double)clock() / CLOCKS_PER_SEC; }
static void bench(void) {
    srand(21);
    printf("-- growth in n (times up to 1e9, durations up to 1e6) --\n");
    for (int n = 100000; n <= 1600000; n *= 2) {
        Mt *m = malloc(n * sizeof(Mt)); int *room = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) { m[i].s = ((long long)rand() << 15 ^ rand()) % 1000000000LL; m[i].e = m[i].s + 1 + rand() % 1000000; m[i].id = i; }
        double t0 = now(); int a = rooms_sweep(m, n); double t1 = now() - t0;
        t0 = now(); int b = assign_rooms(m, n, room); double t2 = now() - t0;
        printf("n=%8d  sweep=%.4f s  heap+schedule=%.4f s  rooms=%d/%d\n", n, t1, t2, a, b);
        free(m); free(room);
    }
    printf("-- difference array when times are small integers (T = 1 000 000) --\n");
    for (int n = 400000; n <= 3200000; n *= 2) {
        Mt *m = malloc(n * sizeof(Mt));
        for (int i = 0; i < n; i++) { m[i].s = rand() % 1000000; m[i].e = m[i].s + 1 + rand() % 100; m[i].id = i; }
        double t0 = now(); int a = rooms_diff(m, n, 1000200); double t1 = now() - t0;
        t0 = now(); int b = rooms_sweep(m, n); double t2 = now() - t0;
        printf("n=%8d  diff-array=%.4f s  sweep=%.4f s  rooms=%d/%d\n", n, t1, t2, a, b);
        free(m);
    }
}

int main(int argc, char **argv) {
    if (argc > 1 && !strcmp(argv[1], "--test"))  return run_tests() ? 1 : 0;
    if (argc > 1 && !strcmp(argv[1], "--bench")) { bench(); return 0; }

    int n;
    if (scanf("%d", &n) != 1 || n < 0) { fprintf(stderr, "bad input\n"); return 1; }
    Mt *m = malloc((n + 1) * sizeof(Mt)); int *room = malloc((n + 1) * sizeof(int));
    for (int i = 0; i < n; i++) {
        if (scanf("%lld %lld", &m[i].s, &m[i].e) != 2 || m[i].s >= m[i].e) { fprintf(stderr, "bad interval %d (need start < end)\n", i + 1); return 1; }
        m[i].id = i;
    }
    int r = rooms_sweep(m, n), r2 = assign_rooms(m, n, room);
    printf("Minimum number of rooms = %d%s\n", r, r == r2 ? "" : "  (INTERNAL MISMATCH)");
    printf("%-8s %-12s %-12s %s\n", "meeting", "start", "end", "room");
    for (int i = 0; i < n; i++) printf("%-8d %-12lld %-12lld %d\n", i + 1, m[i].s, m[i].e, room[i] + 1);
    free(m); free(room);
    return 0;
}
