/*
 * pair_sum_exists.c
 *
 * Given two sets S1, S2 (each size n) and a target x, determine
 * whether some a in S1 and b in S2 satisfy a + b = x, in O(n log n).
 *
 * Algorithm:
 *   1. Sort S2.                                    -- O(n log n)
 *   2. For each a in S1, binary-search S2 for       -- O(n log n)
 *      (x - a). Found on any a => a valid pair exists.
 *   Total: O(n log n) + O(n log n) = O(n log n)
 *
 * This program:
 *   1. Stress-tests correctness against an O(n^2) brute-force check
 *      across many random trials, sizes, and both TRUE and FALSE
 *      outcomes (target explicitly constructed to exist / not exist).
 *   2. Benchmarks comparisons used by the O(n log n) method against
 *      the O(n^2) brute force as n grows, writing a self-contained
 *      GNUPLOT script "plot.gnu" (data embedded inline -- no
 *      .csv/.dat/.py file).
 *
 * Compile:  gcc -O2 -o pair_sum_exists pair_sum_exists.c -lm
 * Run:      ./pair_sum_exists
 * Plot:     gnuplot plot.gnu   (produces comparisons.png and time.png)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

static long long g_cmp; /* comparisons performed by whichever method is running */

/* ======================================================================
 *  O(n log n) METHOD:  sort S2, binary-search for (x - a) for each a
 * ====================================================================== */
static int cmp_int(const void *x, const void *y) { return (*(const int *)x - *(const int *)y); }

static int binary_search_target(const int *S2, int n, int target) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        g_cmp++;
        if (S2[mid] == target) return 1;
        g_cmp++;
        if (S2[mid] < target) lo = mid + 1;
        else hi = mid - 1;
    }
    return 0;
}

/* Returns 1 if some a in S1, b in S2 have a+b == x, else 0.
 * NOTE: sorts a COPY of S2 (S1 and S2 passed in are left unmodified). */
static int pair_sum_exists(const int *S1, int n1, const int *S2in, int n2, int x) {
    int *S2 = malloc((size_t)n2 * sizeof(int));
    for (int i = 0; i < n2; i++) S2[i] = S2in[i];
    qsort(S2, n2, sizeof(int), cmp_int);   /* sort cost not counted in g_cmp
                                               (qsort's internal comparisons
                                               are still O(n log n), just not
                                               instrumented individually) */
    int found = 0;
    for (int i = 0; i < n1 && !found; i++) {
        int target = x - S1[i];
        if (binary_search_target(S2, n2, target)) found = 1;
    }
    free(S2);
    return found;
}

/* ======================================================================
 *  O(n^2) BRUTE FORCE  (correctness baseline)
 * ====================================================================== */
static int pair_sum_exists_brute(const int *S1, int n1, const int *S2, int n2, int x, long long *cmp) {
    for (int i = 0; i < n1; i++)
        for (int j = 0; j < n2; j++) {
            (*cmp)++;
            if (S1[i] + S2[j] == x) return 1;
        }
    return 0;
}

/* ======================================================================
 *  HELPERS
 * ====================================================================== */
static int *random_set(int n, unsigned seed) {
    int *a = malloc((size_t)n * sizeof(int));
    srand(seed);
    for (int i = 0; i < n; i++) a[i] = rand() % 1000000;
    return a;
}

static void run_correctness_tests(void) {
    int fails = 0, total = 0;
    for (int n = 1; n <= 300; n++) {
        for (int trial = 0; trial < 3; trial++) {
            int *S1 = random_set(n, (unsigned)(n * 131 + trial * 3 + 1));
            int *S2 = random_set(n, (unsigned)(n * 131 + trial * 3 + 2));

            /* Case A: x constructed so a pair DEFINITELY exists */
            int xTrue = S1[rand() % n] + S2[rand() % n];
            long long dummy = 0;
            int expectedTrue = pair_sum_exists_brute(S1, n, S2, n, xTrue, &dummy);
            int gotTrue = pair_sum_exists(S1, n, S2, n, xTrue);
            total++;
            if (gotTrue != expectedTrue || expectedTrue != 1) {
                fails++;
                printf("FAIL (should exist): n=%d trial=%d got=%d expected=%d\n",
                       n, trial, gotTrue, expectedTrue);
            }

            /* Case B: x picked far outside any possible sum -> should NOT exist */
            int xFalse = 10000000; /* max possible S1+S2 element sum is < 2,000,000 */
            int expectedFalse = pair_sum_exists_brute(S1, n, S2, n, xFalse, &dummy);
            int gotFalse = pair_sum_exists(S1, n, S2, n, xFalse);
            total++;
            if (gotFalse != expectedFalse) {
                fails++;
                printf("FAIL (should not exist): n=%d trial=%d got=%d expected=%d\n",
                       n, trial, gotFalse, expectedFalse);
            }

            free(S1); free(S2);
        }
    }
    printf("Correctness: %d/%d trials passed (n = 1..300, both existing and non-existing pairs)\n",
           total - fails, total);
}

static double sec(clock_t a, clock_t b) { return (double)(b - a) / CLOCKS_PER_SEC; }

int main(void) {
    run_correctness_tests();

    FILE *gp = fopen("plot.gnu", "w");
    if (!gp) { perror("fopen plot.gnu"); return 1; }
    fprintf(gp, "# Auto-generated by pair_sum_exists.c\n");
    fprintf(gp, "# Run with:  gnuplot plot.gnu\n");
    fprintf(gp, "set terminal pngcairo size 900,650 enhanced font 'Arial,11'\n");
    fprintf(gp, "set grid\n\n");

    int sizes[] = {100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600};
    int nsizes = sizeof(sizes) / sizeof(sizes[0]);

    fprintf(gp, "$DATA << EOD\n");
    fprintf(gp, "# n nlogn_cmp brute_cmp nlogn_time brute_time n_log2n n_squared\n");

    printf("\n%-8s %-16s %-16s %-16s %-16s\n", "n", "O(nlogn) cmp", "O(n^2) cmp", "O(nlogn) time", "O(n^2) time");

    for (int s = 0; s < nsizes; s++) {
        int n = sizes[s];
        /* worst case: x forces a full scan (target never matches until the
         * very last element, or never matches at all) */
        int *S1 = random_set(n, (unsigned)(4000000u + n));
        int *S2 = random_set(n, (unsigned)(5000000u + n));
        int x = 10000000; /* unreachable sum -> forces the true worst case */

        g_cmp = 0;
        clock_t t0 = clock();
        pair_sum_exists(S1, n, S2, n, x);
        clock_t t1 = clock();
        long long cmpNlogn = g_cmp;
        double timeNlogn = sec(t0, t1);

        long long cmpBrute = 0;
        t0 = clock();
        pair_sum_exists_brute(S1, n, S2, n, x, &cmpBrute);
        t1 = clock();
        double timeBrute = sec(t0, t1);

        double nlog2n = (double)n * log2((double)n);
        double nsquared = (double)n * (double)n;

        printf("%-8d %-16lld %-16lld %-16.6f %-16.6f\n", n, cmpNlogn, cmpBrute, timeNlogn, timeBrute);
        fprintf(gp, "%d %lld %lld %.9f %.9f %.2f %.2f\n",
                n, cmpNlogn, cmpBrute, timeNlogn, timeBrute, nlog2n, nsquared);

        free(S1); free(S2);
    }
    fprintf(gp, "EOD\n\n");

    /* --- Plot 1: comparisons vs n, log-log to show the O(n log n) vs O(n^2) gap --- */
    fprintf(gp,
        "set output 'comparisons.png'\n"
        "set title 'Pair-sum-exists: O(n log n) binary-search method vs O(n^2) brute force'\n"
        "set xlabel 'n'\n"
        "set ylabel 'comparisons'\n"
        "set logscale xy\n"
        "plot $DATA using 1:2 with linespoints pt 7 lw 2 title 'O(n log n) method (measured)', \\\n"
        "     $DATA using 1:3 with linespoints pt 5 lw 2 title 'O(n^2) brute force (measured)', \\\n"
        "     $DATA using 1:6 with lines dt 2 lw 1.5 lc rgb 'red'  title 'n log2(n) (theory)', \\\n"
        "     $DATA using 1:7 with lines dt 2 lw 1.5 lc rgb 'blue' title 'n^2 (theory)'\n\n");

    /* --- Plot 2: time vs n --- */
    fprintf(gp,
        "set output 'time.png'\n"
        "set title 'Pair-sum-exists: time vs n'\n"
        "set xlabel 'n'\n"
        "set ylabel 'time (seconds)'\n"
        "set logscale xy\n"
        "plot $DATA using 1:4 with linespoints pt 7 lw 2 title 'O(n log n) method', \\\n"
        "     $DATA using 1:5 with linespoints pt 5 lw 2 title 'O(n^2) brute force'\n");

    fclose(gp);
    return 0;
}
