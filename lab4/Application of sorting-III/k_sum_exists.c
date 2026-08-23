/*
 * k_sum_exists.c
 *
 * Given a set S of n integers, an integer T, and a fixed k, decide
 * whether some k elements of S sum to T, in O(n^(k-1) log n).
 *
 * Algorithm:
 *   1. Sort S.                                             -- O(n log n)
 *   2. Enumerate all C(n,k-1) = O(n^(k-1)) combinations of
 *      (k-1) indices i1 < i2 < ... < i_{k-1} via k-1 nested
 *      loops (implemented here as recursion).
 *   3. For each combination, the required value of the k-th
 *      element is fixed: need = T - (sum of the chosen k-1
 *      elements). Binary-search S for "need".                -- O(log n)
 *   Total: O(n^(k-1)) * O(log n) = O(n^(k-1) log n)
 *
 * (For k=2 this is exactly the classic sort + binary-search 2-SUM
 * algorithm: O(n^1 * log n) = O(n log n).)
 *
 * This program:
 *   1. Stress-tests correctness for k = 2, 3, 4 against an O(n^k)
 *      brute-force check across small n, for both TRUE and FALSE
 *      outcomes.
 *   2. Benchmarks comparisons used by the O(n^(k-1) log n) method
 *      for k=3 and k=4 as n grows (forcing the worst case: T chosen
 *      so that no valid k-tuple exists, preventing any early exit),
 *      writing a self-contained GNUPLOT script "plot.gnu" (data
 *      embedded inline -- no .csv/.dat/.py file).
 *
 * Compile:  gcc -O2 -o k_sum_exists k_sum_exists.c -lm
 * Run:      ./k_sum_exists
 * Plot:     gnuplot plot.gnu   (produces k3.png and k4.png)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define INT_MIN_SAFE (-2000000000LL)
#define INT_MAX_SAFE (2000000000LL)

static long long g_cmp; /* comparisons performed by the O(n^(k-1) log n) method */

static int cmp_int(const void *x, const void *y) { return (*(const int *)x - *(const int *)y); }

/* Binary search for 'target' in sorted S[0..n-1]. If found, verify
 * the matching index isn't already used (relies on S having DISTINCT
 * values in our test inputs, so a value-match implies an index-match
 * against at most one of the already-chosen indices). Returns the
 * found index, or -1. */
static int binary_search_excluding(const int *S, int n, int target,
                                    const int *usedIdx, int usedCount) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        g_cmp++;
        if (S[mid] == target) {
            for (int t = 0; t < usedCount; t++) if (usedIdx[t] == mid) return -1;
            return mid;
        }
        g_cmp++;
        if (S[mid] < target) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

/* Recursively enumerates combinations of (k-1) indices in increasing
 * order; when k-1 have been chosen, binary-searches for the value
 * that would complete the sum to T. */
static int recurse(const int *S, int n, int k, long long T, int startIdx,
                    int chosenCount, long long chosenSum, int *usedIdx) {
    if (chosenCount == k - 1) {
        long long need = T - chosenSum;
        if (need < INT_MIN_SAFE || need > INT_MAX_SAFE) return 0; /* out of int range, can't match */
        int idx = binary_search_excluding(S, n, (int)need, usedIdx, chosenCount);
        return idx >= 0;
    }
    /* need enough remaining elements to fill the rest of the combination */
    int remainingNeeded = (k - 1) - chosenCount;
    for (int i = startIdx; i <= n - remainingNeeded; i++) {
        usedIdx[chosenCount] = i;
        if (recurse(S, n, k, T, i + 1, chosenCount + 1, chosenSum + S[i], usedIdx))
            return 1;
    }
    return 0;
}

/* Sorts a COPY of S (input array left unmodified) and runs the
 * O(n^(k-1) log n) algorithm. */
static int k_sum_exists(const int *Sin, int n, int k, long long T) {
    if (k < 1 || k > n) return 0;
    int *S = malloc((size_t)n * sizeof(int));
    for (int i = 0; i < n; i++) S[i] = Sin[i];
    qsort(S, n, sizeof(int), cmp_int);

    int *usedIdx = malloc((size_t)(k > 1 ? k - 1 : 1) * sizeof(int));
    int result;
    if (k == 1) {
        /* trivial case: does S contain T itself? */
        result = binary_search_excluding(S, n, (int)T, usedIdx, 0) >= 0;
    } else {
        result = recurse(S, n, k, T, 0, 0, 0, usedIdx);
    }
    free(S); free(usedIdx);
    return result;
}

/* ======================================================================
 *  BRUTE FORCE (correctness baseline): try every combination of k
 *  indices directly, O(n^k / k!) roughly (implemented via recursion,
 *  only usable for small n).
 * ====================================================================== */
static int brute_recurse(const int *S, int n, int k, long long T, int startIdx,
                          int chosenCount, long long chosenSum) {
    if (chosenCount == k) return chosenSum == T;
    int remainingNeeded = k - chosenCount;
    for (int i = startIdx; i <= n - remainingNeeded; i++)
        if (brute_recurse(S, n, k, T, i + 1, chosenCount + 1, chosenSum + S[i]))
            return 1;
    return 0;
}
static int k_sum_brute(const int *S, int n, int k, long long T) {
    return brute_recurse(S, n, k, T, 0, 0, 0);
}

/* ======================================================================
 *  HELPERS
 * ====================================================================== */
static int *random_distinct_set(int n, unsigned seed) {
    int *a = malloc((size_t)n * sizeof(int));
    srand(seed);
    for (int i = 0; i < n; i++) a[i] = i * 7 + 1; /* distinct by construction */
    /* shuffle so the array isn't trivially already sorted */
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = a[i]; a[i] = a[j]; a[j] = t;
    }
    return a;
}

static void run_correctness_tests(void) {
    int fails = 0, total = 0;
    int ks[] = {2, 3, 4};
    for (int ki = 0; ki < 3; ki++) {
        int k = ks[ki];
        for (int n = k; n <= 25; n++) {
            for (int trial = 0; trial < 4; trial++) {
                int *S = random_distinct_set(n, (unsigned)(n * 97 + trial * 13 + k));

                /* Case A: T constructed so a valid k-tuple DEFINITELY exists */
                long long sumTrue = 0;
                int used[8] = {0};
                int picked = 0;
                srand((unsigned)(n * 51 + trial + k));
                while (picked < k) {
                    int idx = rand() % n;
                    int dup = 0;
                    for (int t = 0; t < picked; t++) if (used[t] == idx) dup = 1;
                    if (dup) continue;
                    used[picked++] = idx;
                    sumTrue += S[idx];
                }
                int expectedTrue = k_sum_brute(S, n, k, sumTrue);
                int gotTrue = k_sum_exists(S, n, k, sumTrue);
                total++;
                if (gotTrue != expectedTrue || expectedTrue != 1) {
                    fails++;
                    printf("FAIL (should exist): k=%d n=%d trial=%d got=%d expected=%d\n",
                           k, n, trial, gotTrue, expectedTrue);
                }

                /* Case B: T unreachable -> should NOT exist */
                long long tFalse = 5000000000LL;
                int expectedFalse = k_sum_brute(S, n, k, tFalse);
                int gotFalse = k_sum_exists(S, n, k, tFalse);
                total++;
                if (gotFalse != expectedFalse) {
                    fails++;
                    printf("FAIL (should not exist): k=%d n=%d trial=%d got=%d expected=%d\n",
                           k, n, trial, gotFalse, expectedFalse);
                }

                free(S);
            }
        }
    }
    printf("Correctness: %d/%d trials passed (k = 2,3,4; n = k..25; both existing and non-existing sums)\n",
           total - fails, total);
}

static double sec(clock_t a, clock_t b) { return (double)(b - a) / CLOCKS_PER_SEC; }

static void benchmark_for_k(FILE *gp, int k, int *sizes, int nsizes) {
    fprintf(gp, "$DATA_K%d << EOD\n", k);
    fprintf(gp, "# n comparisons time n_pow_km1_logn\n");

    printf("\n=== k = %d ===\n", k);
    printf("%-8s %-16s %-14s %-18s\n", "n", "comparisons", "time(s)", "n^(k-1) log2(n) theory");

    for (int s = 0; s < nsizes; s++) {
        int n = sizes[s];
        int *S = random_distinct_set(n, (unsigned)(9000000u + n * 3 + k));
        /* Unreachable target that stays within int range: the max
         * possible sum of any k elements is at most k*(n*7+1) (see
         * random_distinct_set), so anything comfortably above that
         * is guaranteed unreachable while still forcing every binary
         * search to run to completion (the needed value is always
         * "in range" but never actually present). */
        long long T = (long long)k * (n * 7 + 1) + 1000000;

        g_cmp = 0;
        clock_t t0 = clock();
        int found = k_sum_exists(S, n, k, T);
        clock_t t1 = clock();
        double time = sec(t0, t1);
        (void)found; /* expected to be 0 (not found) -- confirms worst case ran fully */

        double theory = pow((double)n, (double)(k - 1)) * log2((double)n);

        printf("%-8d %-16lld %-14.6f %-18.1f\n", n, g_cmp, time, theory);
        fprintf(gp, "%d %lld %.9f %.2f\n", n, g_cmp, time, theory);

        free(S);
    }
    fprintf(gp, "EOD\n\n");
}

int main(void) {
    run_correctness_tests();

    FILE *gp = fopen("plot.gnu", "w");
    if (!gp) { perror("fopen plot.gnu"); return 1; }
    fprintf(gp, "# Auto-generated by k_sum_exists.c\n");
    fprintf(gp, "# Run with:  gnuplot plot.gnu\n");
    fprintf(gp, "set terminal pngcairo size 900,650 enhanced font 'Arial,11'\n");
    fprintf(gp, "set grid\n");
    fprintf(gp, "set logscale xy\n\n");

    int sizesK3[] = {20, 40, 80, 160, 320, 640, 1280};
    int sizesK4[] = {10, 15, 20, 30, 45, 65, 95};

    benchmark_for_k(gp, 3, sizesK3, sizeof(sizesK3) / sizeof(sizesK3[0]));
    benchmark_for_k(gp, 4, sizesK4, sizeof(sizesK4) / sizeof(sizesK4[0]));

    fprintf(gp,
        "set output 'k3.png'\n"
        "set title 'k=3: comparisons vs n (worst case, no valid triple exists)'\n"
        "set xlabel 'n'\n"
        "set ylabel 'comparisons'\n"
        "plot $DATA_K3 using 1:2 with linespoints pt 7 lw 2 title 'measured (k=3)', \\\n"
        "     $DATA_K3 using 1:4 with lines dt 2 lw 1.5 lc rgb 'red' title 'n^2 log2(n) (theory)'\n\n");

    fprintf(gp,
        "set output 'k4.png'\n"
        "set title 'k=4: comparisons vs n (worst case, no valid quadruple exists)'\n"
        "set xlabel 'n'\n"
        "set ylabel 'comparisons'\n"
        "plot $DATA_K4 using 1:2 with linespoints pt 7 lw 2 title 'measured (k=4)', \\\n"
        "     $DATA_K4 using 1:4 with lines dt 2 lw 1.5 lc rgb 'red' title 'n^3 log2(n) (theory)'\n");

    fclose(gp);
    printf("\nWrote plot.gnu -- run 'gnuplot plot.gnu' to generate k3.png and k4.png\n");
    return 0;
}
