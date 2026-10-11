/*
 * hu_tucker.c  --  Hu-Tucker algorithm for optimal alphabetic binary trees.
 *
 * Input representation: an array w[0..n-1] of non-negative integer weights,
 * kept in the fixed left-to-right (alphabetic) order.  During Phase 1 the
 * working list is an array of records {node id, weight, is_square}; a
 * "square" is an original leaf, a "circle" is a merged (internal) node.
 *
 * Phase 1 (combination): repeatedly merge the minimum-sum COMPATIBLE pair
 *          (no square strictly between them); ties -> leftmost i, then j.
 *          The merged node (a circle) takes the left node's position.
 * Phase 2 (level assignment): depth of every leaf in the combination tree.
 * Phase 3 (recombination): stack scan builds an ORDER-PRESERVING tree
 *          having exactly those leaf depths.
 *
 * Time O(n^2) (O(n) per combination step), space O(n).
 * Validation: O(n^3) interval DP gives the true optimum; compared in --test.
 *
 * Build : gcc -O2 -o hu_tucker hu_tucker.c
 * Run   : ./hu_tucker < input.txt      |      ./hu_tucker --test [trials]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef long long ll;

typedef struct { int id; ll w; int sq; } Item;   /* working-list entry */

/* Computes depth[] of each leaf; returns cost. Arrays are caller-sized. */
static ll hu_tucker_depths(int n, const ll *w, int *depth)
{
    if (n == 1) { depth[0] = 0; return 0; }
    int total = 2 * n - 1;
    int *parent = malloc(total * sizeof(int));
    ll  *nw     = malloc(total * sizeof(ll));
    Item *L     = malloc(n * sizeof(Item));
    int *bj     = malloc(n * sizeof(int));
    int m = n, next = n;

    for (int i = 0; i < n; i++) { L[i].id = i; L[i].w = w[i]; L[i].sq = 1; nw[i] = w[i]; }
    for (int i = 0; i < total; i++) parent[i] = -1;

    /* ---- Phase 1: combination ---- */
    while (m > 1) {
        /* bj[i] = best (min weight, leftmost on ties) partner j>i that is
           compatible with i: j runs up to and including the first square. */
        bj[m - 1] = -1;
        for (int i = m - 2; i >= 0; i--) {
            int c = i + 1;
            if (!L[c].sq && bj[c] != -1 && L[bj[c]].w < L[c].w) c = bj[c];
            bj[i] = c;
        }
        int bi = -1; ll bs = 0;
        for (int i = 0; i < m - 1; i++) {
            ll s = L[i].w + L[bj[i]].w;
            if (bi < 0 || s < bs) { bi = i; bs = s; }   /* strict < : leftmost i */
        }
        int j = bj[bi];
        int a = L[bi].id, b = L[j].id;
        parent[a] = parent[b] = next;
        nw[next] = bs;
        L[bi].id = next++; L[bi].w = bs; L[bi].sq = 0;
        memmove(&L[j], &L[j + 1], (m - j - 1) * sizeof(Item));
        m--;
    }

    /* ---- Phase 2: level numbers (parents have larger ids than children) ---- */
    int *dep = calloc(total, sizeof(int));
    for (int v = total - 2; v >= 0; v--) dep[v] = dep[parent[v]] + 1;
    ll cost = 0;
    for (int i = 0; i < n; i++) { depth[i] = dep[i]; cost += w[i] * depth[i]; }

    free(parent); free(nw); free(L); free(bj); free(dep);
    return cost;
}

/* ---- Phase 3: rebuild an alphabetic tree from leaf depths ---- */
typedef struct { int l, r; } TNode;       /* leaf i: id i (<n); internal: id >= n */

static int build_tree(int n, const int *depth, TNode *t)
{
    int *stk = malloc(n * sizeof(int)), *lev = malloc(n * sizeof(int));
    int sp = 0, next = n;
    for (int i = 0; i < n; i++) {
        stk[sp] = i; lev[sp] = depth[i]; sp++;
        while (sp >= 2 && lev[sp - 1] == lev[sp - 2]) {
            t[next].l = stk[sp - 2]; t[next].r = stk[sp - 1];
            sp--; stk[sp - 1] = next++; lev[sp - 1]--;
        }
    }
    int root = (sp == 1 && lev[0] == 0) ? stk[0] : -1;
    free(stk); free(lev);
    return root;
}

static void print_tree(int v, int n, const TNode *t)
{
    if (v < n) { printf("%d", v + 1); return; }
    printf("("); print_tree(t[v].l, n, t); printf(" "); print_tree(t[v].r, n, t); printf(")");
}

/* in-order leaf check + cost of the rebuilt tree (iterative DFS) */
static int verify_tree(int root, int n, const TNode *t, const ll *w, ll *cost)
{
    int *st = malloc((2 * n) * sizeof(int)), *dp = malloc((2 * n) * sizeof(int));
    int sp = 0, expect = 0, ok = 1; *cost = 0;
    st[sp] = root; dp[sp++] = 0;
    while (sp) {
        int v = st[--sp], d = dp[sp];
        if (v < n) { if (v != expect++) ok = 0; *cost += w[v] * d; }
        else { st[sp] = t[v].r; dp[sp++] = d + 1; st[sp] = t[v].l; dp[sp++] = d + 1; }
    }
    free(st); free(dp);
    return ok && expect == n;
}

/* ---- reference: optimal alphabetic tree by interval DP, O(n^3) ---- */
static ll dp_optimal(int n, const ll *w)
{
    ll *c = calloc((size_t)n * n, sizeof(ll)), *pre = calloc(n + 1, sizeof(ll));
    for (int i = 0; i < n; i++) pre[i + 1] = pre[i] + w[i];
    for (int len = 2; len <= n; len++)
        for (int i = 0; i + len <= n; i++) {
            int j = i + len - 1; ll best = -1;
            for (int k = i; k < j; k++) {
                ll v = c[i * n + k] + c[(k + 1) * n + j];
                if (best < 0 || v < best) best = v;
            }
            c[i * n + j] = best + pre[j + 1] - pre[i];
        }
    ll r = c[n - 1]; free(c); free(pre); return r;
}

static int run_tests(int trials)
{
    srand(12345); int bad = 0;
    for (int t = 0; t < trials; t++) {
        int n = 1 + rand() % 40, maxw = (rand() % 3 == 0) ? 3 : 1000;
        ll *w = malloc(n * sizeof(ll)); int *d = malloc(n * sizeof(int));
        TNode *tr = malloc((2 * n) * sizeof(TNode));
        for (int i = 0; i < n; i++) w[i] = rand() % (maxw + 1);
        ll ht = hu_tucker_depths(n, w, d), opt = dp_optimal(n, w), c2;
        int root = build_tree(n, d, tr);
        int ok = root >= 0 && verify_tree(root, n, tr, w, &c2) && c2 == ht && ht == opt;
        if (!ok) { bad++; printf("FAIL n=%d HT=%lld DP=%lld\n", n, ht, opt); }
        free(w); free(d); free(tr);
    }
    printf("%d random tests, %d failures\n", trials, bad);
    return bad != 0;
}

int main(int argc, char **argv)
{
    if (argc > 1 && !strcmp(argv[1], "--test"))
        return run_tests(argc > 2 ? atoi(argv[2]) : 2000);

    int n;
    if (scanf("%d", &n) != 1 || n < 1) { fprintf(stderr, "bad input\n"); return 1; }
    ll *w = malloc(n * sizeof(ll)); int *d = malloc(n * sizeof(int));
    TNode *tr = malloc((2 * n) * sizeof(TNode));
    for (int i = 0; i < n; i++)
        if (scanf("%lld", &w[i]) != 1) { fprintf(stderr, "bad input\n"); return 1; }

    ll cost = hu_tucker_depths(n, w, d);
    int root = build_tree(n, d, tr);
    ll c2 = 0;
    int ok = root >= 0 && verify_tree(root, n, tr, w, &c2);

    printf("Leaf depths :");
    for (int i = 0; i < n; i++) printf(" %d", d[i]);
    printf("\nTree        : "); if (root >= 0) print_tree(root, n, tr); printf("\n");
    printf("Hu-Tucker cost : %lld\n", cost);
    if (n <= 400) {
        ll opt = dp_optimal(n, w);
        printf("DP optimum     : %lld  [%s]\n", opt, opt == cost ? "MATCH" : "MISMATCH");
    }
    printf("In-order preserved & cost recomputed: %s (%lld)\n", ok ? "OK" : "FAILED", c2);
    free(w); free(d); free(tr);
    return 0;
}
