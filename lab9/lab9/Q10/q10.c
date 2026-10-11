/*
 * Q10 - Greedy Superstring Conjecture  (DAA Lab 09)
 *
 * Problem   : given strings S = {s_1..s_n}, find a shortest string containing
 *             every s_i as a substring (NP-hard).
 * Conjecture: the classic GREEDY algorithm (repeatedly merge the two strings
 *             with the largest overlap) is a 2-approximation.
 *
 * What this program does
 *   - removes duplicates and strings that are substrings of other strings;
 *   - runs the GREEDY maximum-overlap merging algorithm;
 *   - computes the TRUE optimum with a Held-Karp bitmask DP over orderings
 *     (exact, n <= 12 strings after reduction);
 *   - reports |greedy|, |optimal| and the ratio |greedy| / |optimal|, so that
 *     the conjectured bound (ratio <= 2) can be tested experimentally.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_STR 12
#define MAX_LEN 24
#define BUF 512

typedef struct {
    int n;
    char s[MAX_STR][MAX_LEN + 2];
} StringSet;

/* ------------------------------------------------------------------ */
static int read_int(const char *prompt, int lo, int hi) {
    int val;
    for (;;) {
        printf("%s", prompt);
        if (scanf("%d", &val) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            if (c == EOF) exit(1);
            printf("  Invalid input, try again.\n");
            continue;
        }
        if (val < lo || val > hi) { printf("  Value must be in [%d, %d].\n", lo, hi); continue; }
        return val;
    }
}

static int choose_input_method(void) {
    printf("Select input method:\n");
    printf("  1. Manual user input\n");
    printf("  2. Randomized data generation\n");
    return read_int("Enter choice (1/2): ", 1, 2);
}

/* ------------------------------------------------------------------ */
/* input                                                               */
/* ------------------------------------------------------------------ */
static void input_manual(StringSet *in) {
    in->n = read_int("Number of strings n (1..12): ", 1, MAX_STR);
    printf("Enter %d non-empty strings without spaces (length <= %d):\n", in->n, MAX_LEN);
    for (int i = 0; i < in->n; i++) {
        for (;;) {
            char buf[256];
            printf("  string %d: ", i + 1);
            if (scanf("%255s", buf) != 1) exit(1);
            if (strlen(buf) >= 1 && strlen(buf) <= MAX_LEN) { strcpy(in->s[i], buf); break; }
            printf("  Length must be between 1 and %d.\n", MAX_LEN);
        }
    }
}

/* Constraints: n in [3,8], alphabet {a,b} or {a,b,c}, each length in [2,6]. */
static void input_random(StringSet *in) {
    in->n = 3 + rand() % 6;
    int alpha = 2 + rand() % 2;
    for (int i = 0; i < in->n; i++) {
        int len = 2 + rand() % 5;
        for (int j = 0; j < len; j++) in->s[i][j] = (char)('a' + rand() % alpha);
        in->s[i][len] = '\0';
    }
}

/* ------------------------------------------------------------------ */
/* helpers                                                             */
/* ------------------------------------------------------------------ */
/* longest proper overlap: suffix of a == prefix of b, length < min(|a|,|b|) */
static int overlap(const char *a, const char *b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    int m = (la < lb ? la : lb) - 1;
    for (int l = m; l >= 1; l--)
        if (strncmp(a + la - l, b, (size_t)l) == 0) return l;
    return 0;
}

/* drop duplicates and strings contained in another string */
static void reduce_set(const StringSet *in, StringSet *out) {
    out->n = 0;
    for (int i = 0; i < in->n; i++) {
        int drop = 0;
        for (int j = 0; j < in->n && !drop; j++) {
            if (i == j) continue;
            if (strstr(in->s[j], in->s[i]) != NULL) {
                if (strcmp(in->s[i], in->s[j]) != 0) drop = 1; /* proper substring */
                else if (j < i) drop = 1;                       /* duplicate        */
            }
        }
        if (!drop) strcpy(out->s[out->n++], in->s[i]);
    }
}

/* ------------------------------------------------------------------ */
/* greedy algorithm                                                    */
/* ------------------------------------------------------------------ */
static void greedy_superstring(const StringSet *r, char *result) {
    int n = r->n;
    static char buf[MAX_STR][BUF];
    int ov[MAX_STR][MAX_STR], active[MAX_STR];
    for (int i = 0; i < n; i++) { strcpy(buf[i], r->s[i]); active[i] = 1; }
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) ov[i][j] = (i == j) ? 0 : overlap(r->s[i], r->s[j]);

    for (int merges = 0; merges < n - 1; merges++) {
        int bi = -1, bj = -1, best = -1;
        for (int i = 0; i < n; i++) {
            if (!active[i]) continue;
            for (int j = 0; j < n; j++) {
                if (i == j || !active[j]) continue;
                if (ov[i][j] > best) { best = ov[i][j]; bi = i; bj = j; }
            }
        }
        /* merge string bj into bi:  buf[bi] + buf[bj] without the overlap */
        char merged[BUF];
        snprintf(merged, sizeof merged, "%s%s", buf[bi], buf[bj] + best);
        strcpy(buf[bi], merged);
        active[bj] = 0;
        /* overlaps of the merged string: its suffix is bj's suffix, its prefix is bi's prefix */
        for (int x = 0; x < n; x++) {
            if (!active[x] || x == bi) continue;
            ov[bi][x] = ov[bj][x];
        }
    }
    for (int i = 0; i < n; i++)
        if (active[i]) { strcpy(result, buf[i]); return; }
    result[0] = '\0';
}

/* ------------------------------------------------------------------ */
/* exact optimum: Held-Karp over the order of the strings               */
/* ------------------------------------------------------------------ */
static void optimal_superstring(const StringSet *r, char *result) {
    int n = r->n;
    static int dp[1 << MAX_STR][MAX_STR], par[1 << MAX_STR][MAX_STR];
    int ov[MAX_STR][MAX_STR];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) ov[i][j] = (i == j) ? 0 : overlap(r->s[i], r->s[j]);

    int full = (1 << n) - 1;
    for (int m = 0; m <= full; m++)
        for (int i = 0; i < n; i++) { dp[m][i] = -1; par[m][i] = -1; }
    for (int i = 0; i < n; i++) dp[1 << i][i] = 0;

    for (int m = 1; m <= full; m++)
        for (int i = 0; i < n; i++) {
            if (dp[m][i] < 0) continue;
            for (int j = 0; j < n; j++) {
                if (m & (1 << j)) continue;
                int nm = m | (1 << j), v = dp[m][i] + ov[i][j];
                if (v > dp[nm][j]) { dp[nm][j] = v; par[nm][j] = i; }
            }
        }

    int last = 0;
    for (int i = 1; i < n; i++) if (dp[full][i] > dp[full][last]) last = i;

    int order[MAX_STR], cnt = 0, m = full, cur = last;
    while (cur >= 0) {
        order[cnt++] = cur;
        int p = par[m][cur];
        m ^= (1 << cur);
        cur = p;
    }
    strcpy(result, r->s[order[cnt - 1]]);
    for (int k = cnt - 2; k >= 0; k--) {
        int o = ov[order[k + 1]][order[k]]; /* overlap with the previous string */
        strcat(result, r->s[order[k]] + o);
    }
}

static int contains_all(const StringSet *in, const char *sup) {
    for (int i = 0; i < in->n; i++)
        if (strstr(sup, in->s[i]) == NULL) return 0;
    return 1;
}

/* ------------------------------------------------------------------ */
int main(void) {
    StringSet in, red;
    char greedy[BUF], opt[BUF];
    srand((unsigned)time(NULL) ^ (unsigned)clock());

    printf("=== Q10: Greedy Superstring Conjecture ===\n");
    if (choose_input_method() == 1) input_manual(&in);
    else input_random(&in);

    printf("\nInput strings (n = %d):", in.n);
    for (int i = 0; i < in.n; i++) printf(" %s", in.s[i]);
    printf("\n");

    reduce_set(&in, &red);
    printf("After removing duplicates / substrings (n = %d):", red.n);
    for (int i = 0; i < red.n; i++) printf(" %s", red.s[i]);
    printf("\n");

    greedy_superstring(&red, greedy);
    optimal_superstring(&red, opt);

    int lg = (int)strlen(greedy), lo = (int)strlen(opt);
    printf("\nGreedy superstring  (length %2d): %s\n", lg, greedy);
    printf("Optimal superstring (length %2d): %s\n", lo, opt);
    printf("Approximation ratio |greedy| / |optimal| = %.4f\n", (double)lg / (double)lo);
    printf("Conjectured bound: ratio <= 2   -> %s on this instance\n", lg <= 2 * lo ? "holds" : "VIOLATED");

    printf("\nValidation\n");
    printf("  greedy result contains every input string  : %s\n", contains_all(&in, greedy) ? "PASSED" : "FAILED");
    printf("  optimal result contains every input string : %s\n", contains_all(&in, opt) ? "PASSED" : "FAILED");
    printf("  |optimal| <= |greedy|                      : %s\n", lo <= lg ? "PASSED" : "FAILED");

    // printf("\nNote: the handout reports an (as yet unvalidated) September 2026 arXiv claim that the\n");
    // printf("greedy ratio can reach (9k+2)/(4k+4) -> 9/4 for even string length k >= 10. This program\n");
    // printf("only measures ratios on instances you give it; it does not prove or disprove the claim.\n");
    return 0;
}
