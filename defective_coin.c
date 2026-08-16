/*
 * defective_coin.c
 *
 * Divide-and-conquer algorithm to find a single lighter (defective)
 * coin among n coins using a balance scale -- or determine that no
 * defective coin exists. Uses at most ceil(log2 n) + 1 weighings.
 *
 * Algorithm (see accompanying explanation):
 *   Maintain a candidate set S (initially all n coins) with the
 *   invariant: any coin outside S is confirmed genuine, and if a
 *   defective coin exists at all, it must be inside S.
 *     - |S| == 1  -> that coin is defective (proved by the previous
 *                    level's weighing already, no extra weighing).
 *     - |S| even  -> split into two equal halves, weigh them.
 *                    balanced -> no defective; unbalanced -> recurse
 *                    into the lighter half.
 *     - |S| odd   -> set one coin aside, split the rest (even) into
 *                    two equal halves, weigh them. balanced -> the
 *                    set-aside coin is the only remaining suspect,
 *                    resolved with exactly one more weighing against
 *                    a known-genuine coin. unbalanced -> recurse into
 *                    the lighter half.
 *
 * This program:
 *   1. Stress-tests correctness across thousands of random trials
 *      (with and without a planted defective coin).
 *   2. Benchmarks the number of weighings used as n grows, and
 *      writes a self-contained GNUPLOT script "plot.gnu" (data
 *      embedded inline -- no .csv/.dat/.py file) comparing the
 *      measured weighings against the ceil(log2 n)+1 theoretical
 *      bound.
 *
 * Compile:  gcc -O2 -o defective_coin defective_coin.c -lm
 * Run:      ./defective_coin
 * Plot:     gnuplot plot.gnu   (produces weighings.png)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

typedef struct { int origIndex; int weight; } Coin;

static long long g_weighings;  /* reset before every solve() call */

/* The ONLY primitive allowed: compare total weight of a[lo1,hi1)
 * against a[lo2,hi2). Returns -1 if the first group is lighter,
 * +1 if the second group is lighter, 0 if they balance. */
static int weigh(const Coin *a, int lo1, int hi1, int lo2, int hi2) {
    g_weighings++;
    long s1 = 0, s2 = 0;
    for (int i = lo1; i < hi1; i++) s1 += a[i].weight;
    for (int i = lo2; i < hi2; i++) s2 += a[i].weight;
    if (s1 < s2) return -1;
    if (s1 > s2) return 1;
    return 0;
}

/* Returns the original index of the defective coin within a[lo,hi),
 * or -1 if none exists in this range. Relies on the invariant
 * described above: if a defective coin exists anywhere in the
 * original input, it is guaranteed to lie within [lo,hi). */
static int solve(const Coin *a, int lo, int hi) {
    int m = hi - lo;
    if (m <= 0) return -1;
    if (m == 1) return a[lo].origIndex;   /* proved defective by the caller's weighing */

    if (m % 2 == 0) {
        int mid = lo + m / 2;
        int r = weigh(a, lo, mid, mid, hi);
        if (r == 0) return -1;                    /* balanced: no defective in here */
        return (r < 0) ? solve(a, lo, mid) : solve(a, mid, hi);
    } else {
        int asideIdx = hi - 1;                    /* set this one coin aside */
        int evenLo = lo, evenHi = hi - 1;          /* remaining m-1 coins (even) */
        int mid = evenLo + (evenHi - evenLo) / 2;
        int r = weigh(a, evenLo, mid, mid, evenHi);
        if (r == 0) {
            /* R1, R2 both confirmed genuine -> only the set-aside coin
             * is still a suspect. One final check against a coin we
             * just proved genuine (a[evenLo]). */
            g_weighings++;
            return (a[asideIdx].weight < a[evenLo].weight) ? a[asideIdx].origIndex : -1;
        }
        return (r < 0) ? solve(a, evenLo, mid) : solve(a, mid, evenHi);
    }
}

/* ======================================================================
 *  TEST / BENCHMARK HARNESS
 * ====================================================================== */

static const int NORMAL_WEIGHT = 1000;

/* Build n coins; if defectivePos >= 0, that coin (by position 0..n-1)
 * is made 1 unit lighter than normal. Coins are then shuffled so the
 * defective coin's position within the array is randomized -- only
 * origIndex still identifies which original coin it is. */
static Coin *build_coins(int n, int defectivePos, unsigned seed) {
    Coin *a = malloc((size_t)n * sizeof(Coin));
    for (int i = 0; i < n; i++) {
        a[i].origIndex = i;
        a[i].weight = (i == defectivePos) ? NORMAL_WEIGHT - 1 : NORMAL_WEIGHT;
    }
    srand(seed);
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Coin t = a[i]; a[i] = a[j]; a[j] = t;
    }
    return a;
}

static void run_correctness_tests(void) {
    int fails = 0, total = 0;
    for (int n = 2; n <= 500; n++) {
        for (int trial = 0; trial < 5; trial++) {
            /* Case 1: a genuine defective coin exists */
            int defPos = rand() % n;
            Coin *a = build_coins(n, defPos, (unsigned)(n * 97 + trial));
            g_weighings = 0;
            int found = solve(a, 0, n);
            int expectedOrigIndex = -1;
            for (int i = 0; i < n; i++) if (a[i].weight < NORMAL_WEIGHT) expectedOrigIndex = a[i].origIndex;
            total++;
            if (found != expectedOrigIndex) {
                fails++;
                printf("FAIL (defective present): n=%d trial=%d found=%d expected=%d\n",
                       n, trial, found, expectedOrigIndex);
            }
            free(a);

            /* Case 2: no defective coin at all */
            Coin *b = build_coins(n, -1, (unsigned)(n * 131 + trial));
            g_weighings = 0;
            int found2 = solve(b, 0, n);
            total++;
            if (found2 != -1) {
                fails++;
                printf("FAIL (no defective): n=%d trial=%d found=%d expected=-1\n", n, trial, found2);
            }
            free(b);
        }
    }
    printf("Correctness: %d/%d trials passed (n = 2..500, with and without a defective coin)\n",
           total - fails, total);
}

int main(void) {
    run_correctness_tests();

    int sizes[] = {2, 3, 4, 5, 7, 8, 16, 17, 32, 33, 64, 100, 128, 250, 256,
                   500, 512, 777, 1000, 1024, 4096, 12345, 16384,
                   65536, 100000, 262144};
    int nsizes = sizeof(sizes) / sizeof(sizes[0]);

    FILE *gp = fopen("plot.gnu", "w");
    if (!gp) { perror("fopen plot.gnu"); return 1; }
    fprintf(gp, "# Auto-generated by defective_coin.c\n");
    fprintf(gp, "# Run with:  gnuplot plot.gnu\n");
    fprintf(gp, "set terminal pngcairo size 900,650 enhanced font 'Arial,11'\n");
    fprintf(gp, "set grid\n");
    fprintf(gp, "set logscale x\n");
    fprintf(gp, "set xlabel 'n (number of coins)'\n");
    fprintf(gp, "set ylabel 'number of weighings'\n");
    fprintf(gp, "set output 'weighings.png'\n");
    fprintf(gp, "set title 'Defective coin search: weighings vs n'\n\n");

    fprintf(gp, "$DATA << EOD\n");
    fprintf(gp, "# n max_weighings_with_defect max_weighings_no_defect ceil_log2n_plus_1\n");

    printf("\nn         max_weighings(defect present)   max_weighings(no defect)   ceil(log2 n)+1\n");

    for (int s = 0; s < nsizes; s++) {
        int n = sizes[s];
        long long maxWithDefect = 0, maxNoDefect = 0;

        /* try EVERY possible defective position (or a large sample for
         * big n) to find the true worst case number of weighings */
        int trials = (n <= 4096) ? n : 4096;
        for (int t = 0; t < trials; t++) {
            int defPos = (n <= 4096) ? t : (rand() % n);
            Coin *a = build_coins(n, defPos, (unsigned)(1000000u + n * 17u + t));
            g_weighings = 0;
            solve(a, 0, n);
            if (g_weighings > maxWithDefect) maxWithDefect = g_weighings;
            free(a);
        }
        for (int t = 0; t < 20; t++) {
            Coin *b = build_coins(n, -1, (unsigned)(2000000u + n * 31u + t));
            g_weighings = 0;
            solve(b, 0, n);
            if (g_weighings > maxNoDefect) maxNoDefect = g_weighings;
            free(b);
        }

        double bound = ceil(log2((double)n)) + 1.0;
        printf("%-9d %-30lld %-27lld %.0f\n", n, maxWithDefect, maxNoDefect, bound);
        fprintf(gp, "%d %lld %lld %.2f\n", n, maxWithDefect, maxNoDefect, bound);
    }
    fprintf(gp, "EOD\n\n");

    fprintf(gp,
        "plot $DATA using 1:2 with linespoints pt 7 lw 2 title 'worst-case weighings (defect present)', \\\n"
        "     $DATA using 1:3 with linespoints pt 5 lw 2 title 'worst-case weighings (no defect)', \\\n"
        "     $DATA using 1:4 with lines dt 2 lw 1.5 lc rgb 'red' title 'ceil(log2 n) + 1 (theoretical bound)'\n");

    fclose(gp);
    printf("\nWrote plot.gnu -- run 'gnuplot plot.gnu' to generate weighings.png\n");
    return 0;
}
