/*
 * merge_intervals.c
 *
 * Given n intervals (x_i, y_i), merge all overlapping ones. E.g.
 * {(1,3),(2,6),(8,10),(7,18)} -> {(1,6),(7,18)}.
 *
 * Algorithm:
 *   1. Sort intervals by start value x_i.        -- O(n log n)
 *   2. Single pass: keep a "current" merged interval. For each next
 *      interval, if its start <= current's end, extend current's end
 *      to max(current.end, next.end); otherwise close off current
 *      and start a new one with next.             -- O(n)
 *   Total: O(n log n)
 *
 * Convention: touching endpoints count as overlapping, i.e. (1,3)
 * and (3,5) merge into (1,5), since intervals are treated as closed
 * [x,y]. This matches the given example (7,18)+(8,10) -> (7,18).
 *
 * This program:
 *   1. Stress-tests correctness against an independent "coverage
 *      array" ground truth (mark every integer point covered by any
 *      interval, then read off the resulting contiguous runs) on a
 *      bounded integer range, across many random trials and sizes.
 *   2. Benchmarks the O(n log n) method's sort-comparisons against a
 *      naive O(n^2) repeated-pass merge as n grows, writing a
 *      self-contained GNUPLOT script "plot.gnu" 
 *
 * Compile:  gcc -O2 -o merge_intervals merge_intervals.c -lm
 * Run:      ./merge_intervals
 * Plot:     gnuplot plot.gnu   (produces comparisons.png and time.png)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

typedef struct { long long x, y; } Interval;

static long long g_cmp; /* comparisons made while sorting */

static int cmp_interval(const void *p, const void *q) {
    g_cmp++;
    const Interval *a = p, *b = q;
    if (a->x < b->x) return -1;
    if (a->x > b->x) return 1;
    return 0;
}

/* ======================================================================
 *  O(n log n) MERGE
 *  Returns the number of merged intervals; fills 'out' (caller must
 *  provide an array of at least n Intervals).
 * ====================================================================== */
static int merge_intervals_fast(const Interval *in, int n, Interval *out) {
    if (n == 0) return 0;
    Interval *a = malloc((size_t)n * sizeof(Interval));
    memcpy(a, in, (size_t)n * sizeof(Interval));
    qsort(a, n, sizeof(Interval), cmp_interval);

    int m = 0;
    out[0] = a[0];
    for (int i = 1; i < n; i++) {
        if (a[i].x <= out[m].y) {
            if (a[i].y > out[m].y) out[m].y = a[i].y;
        } else {
            out[++m] = a[i];
        }
    }
    free(a);
    return m + 1;
}

/* ======================================================================
 *  NAIVE O(n^2) (repeated-pass) MERGE -- used only as a comparison
 *  baseline for the benchmark, not for correctness (correctness uses
 *  the independent coverage-array method below).
 * ====================================================================== */
static int merge_intervals_naive(const Interval *in, int n, Interval *out, long long *cmp) {
    Interval *a = malloc((size_t)n * sizeof(Interval));
    memcpy(a, in, (size_t)n * sizeof(Interval));
    int m = n;
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < m && !changed; i++) {
            for (int j = i + 1; j < m; j++) {
                (*cmp)++;
                int overlap = (a[i].x <= a[j].y) && (a[j].x <= a[i].y);
                if (overlap) {
                    if (a[j].x < a[i].x) a[i].x = a[j].x;
                    if (a[j].y > a[i].y) a[i].y = a[j].y;
                    a[j] = a[--m]; /* remove j by swapping with the last */
                    changed = 1;
                    break;
                }
            }
        }
    }
    memcpy(out, a, (size_t)m * sizeof(Interval));
    free(a);
    /* sort the (few, now non-overlapping) results by start for a
     * canonical, comparable output */
    qsort(out, m, sizeof(Interval), cmp_interval);
    return m;
}

/* ======================================================================
 *  CORRECTNESS GROUND TRUTH: coverage array over a bounded integer
 *  range. Mark every point covered by any input interval (closed
 *  [x,y]), then read off maximal contiguous covered runs.
 *
 *  NOTE: coordinates are doubled internally before marking. This is
 *  essential: with plain integer sampling, two truly non-overlapping,
 *  non-touching intervals like (1,3) and (4,6) have NO integer point
 *  strictly between them (nothing between 3 and 4), so a naive
 *  integer coverage array would wrongly see them as contiguous and
 *  merge them. Doubling first turns that gap into the doubled points
 *  6 and 8 with 7 genuinely uncovered in between, correctly
 *  preserving the gap.
 * ====================================================================== */
static int merge_intervals_ground_truth(const Interval *in, int n, int range, Interval *out) {
    int scaledRange = 2 * range + 2;
    char *covered = calloc((size_t)(scaledRange + 1), 1);
    for (int i = 0; i < n; i++)
        for (long long p = 2 * in[i].x; p <= 2 * in[i].y; p++)
            covered[p] = 1;

    int m = 0;
    int p = 0;
    while (p <= scaledRange) {
        if (covered[p]) {
            int start = p;
            while (p <= scaledRange && covered[p]) p++;
            out[m].x = start / 2; out[m].y = (p - 1) / 2; m++;
        } else p++;
    }
    free(covered);
    return m;
}

/* ======================================================================
 *  HELPERS
 * ====================================================================== */
static void make_chain_intervals(int n, unsigned seed, Interval *out) {
    /* A "chain" where interval i overlaps interval i+1 by exactly 1
     * unit: (0,2),(1,3),(2,4),...  Everything eventually merges into
     * one giant interval, but only by combining ONE adjacent pair at
     * a time -- this is the pathological case for a repeated-pass
     * merge algorithm, since it can only ever discover and merge one
     * overlapping pair before having to rescan. The chain is then
     * shuffled so the naive algorithm can't discover merges "for
     * free" in a convenient left-to-right order. */
    for (int i = 0; i < n; i++) { out[i].x = i; out[i].y = i + 2; }
    srand(seed);
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Interval t = out[i]; out[i] = out[j]; out[j] = t;
    }
}

static void make_random_intervals(int n, int maxCoord, unsigned seed, Interval *out) {
    srand(seed);
    for (int i = 0; i < n; i++) {
        int x = rand() % (maxCoord + 1);
        int len = rand() % (maxCoord / 4 + 1);
        out[i].x = x;
        out[i].y = x + len;
        if (out[i].y > maxCoord) out[i].y = maxCoord;
    }
}

static int intervals_equal(const Interval *a, int na, const Interval *b, int nb) {
    if (na != nb) return 0;
    for (int i = 0; i < na; i++) if (a[i].x != b[i].x || a[i].y != b[i].y) return 0;
    return 1;
}

static void run_correctness_tests(void) {
    int fails = 0, total = 0;
    int range = 300;
    for (int n = 0; n <= 200; n++) {
        for (int trial = 0; trial < 3; trial++) {
            Interval *in = malloc((size_t)(n > 0 ? n : 1) * sizeof(Interval));
            make_random_intervals(n, range, (unsigned)(n * 97 + trial), in);

            Interval *outFast = malloc((size_t)(n > 0 ? n : 1) * sizeof(Interval));
            Interval *outTruth = malloc((size_t)(range + 2) * sizeof(Interval));

            int mFast = merge_intervals_fast(in, n, outFast);
            /* canonical order: sort outFast by x for comparison (it's
             * already sorted by construction, but be explicit/safe) */
            qsort(outFast, mFast, sizeof(Interval), cmp_interval);

            int mTruth = merge_intervals_ground_truth(in, n, range, outTruth);

            total++;
            if (!intervals_equal(outFast, mFast, outTruth, mTruth)) {
                fails++;
                printf("FAIL: n=%d trial=%d  fast_m=%d truth_m=%d\n", n, trial, mFast, mTruth);
            }

            free(in); free(outFast); free(outTruth);
        }
    }

    /* also check the exact example from the problem statement */
    Interval example[] = {{1,3},{2,6},{8,10},{7,18}};
    Interval expected[] = {{1,6},{7,18}};
    Interval outEx[4];
    int mEx = merge_intervals_fast(example, 4, outEx);
    total++;
    if (!intervals_equal(outEx, mEx, expected, 2)) {
        fails++;
        printf("FAIL: worked example did not match expected output\n");
    }

    printf("Correctness: %d/%d trials passed (n = 0..200, verified against an independent "
           "coverage-array ground truth; worked example also checked)\n", total - fails, total);
}

static double sec(clock_t a, clock_t b) { return (double)(b - a) / CLOCKS_PER_SEC; }

int main(void) {
    run_correctness_tests();

    FILE *gp = fopen("plot.gnu", "w");
    if (!gp) { perror("fopen plot.gnu"); return 1; }
    fprintf(gp, "# Auto-generated by merge_intervals.c\n");
    fprintf(gp, "# Run with:  gnuplot plot.gnu\n");
    fprintf(gp, "set terminal pngcairo size 900,650 enhanced font 'Arial,11'\n");
    fprintf(gp, "set grid\n\n");

    int sizes[] = {50, 100, 200, 400, 800, 1600, 3200};
    int nsizes = sizeof(sizes) / sizeof(sizes[0]);

    fprintf(gp, "$DATA << EOD\n");
    fprintf(gp, "# n fast_cmp naive_cmp fast_time naive_time n_log2n n_squared\n");

    printf("\n%-8s %-14s %-14s %-14s %-14s\n", "n", "fast cmp", "naive cmp", "fast time", "naive time");

    for (int s = 0; s < nsizes; s++) {
        int n = sizes[s];
        Interval *in = malloc((size_t)n * sizeof(Interval));
        /* shuffled overlap-chain: pathological worst case for the
         * naive repeated-pass method (see make_chain_intervals) */
        make_chain_intervals(n, (unsigned)(9000000u + n), in);

        Interval *outFast = malloc((size_t)n * sizeof(Interval));
        Interval *outNaive = malloc((size_t)n * sizeof(Interval));

        g_cmp = 0;
        clock_t t0 = clock();
        merge_intervals_fast(in, n, outFast);
        clock_t t1 = clock();
        double timeFast = sec(t0, t1);
        long long cmpFast = g_cmp;

        long long cmpNaive = 0;
        t0 = clock();
        merge_intervals_naive(in, n, outNaive, &cmpNaive);
        t1 = clock();
        double timeNaive = sec(t0, t1);

        double nlog2n = (double)n * log2((double)n);
        double nsquared = (double)n * (double)n;

        printf("%-8d %-14lld %-14lld %-14.6f %-14.6f\n", n, cmpFast, cmpNaive, timeFast, timeNaive);
        fprintf(gp, "%d %lld %lld %.9f %.9f %.2f %.2f\n",
                n, cmpFast, cmpNaive, timeFast, timeNaive, nlog2n, nsquared);

        free(in); free(outFast); free(outNaive);
    }
    fprintf(gp, "EOD\n\n");

    fprintf(gp,
        "set output 'comparisons.png'\n"
        "set title 'Merge intervals: O(n log n) sort-based vs O(n^2) naive repeated-pass'\n"
        "set xlabel 'n (number of intervals)'\n"
        "set ylabel 'comparisons'\n"
        "set logscale xy\n"
        "plot $DATA using 1:2 with linespoints pt 7 lw 2 title 'sort-based (measured)', \\\n"
        "     $DATA using 1:3 with linespoints pt 5 lw 2 title 'naive repeated-pass (measured)', \\\n"
        "     $DATA using 1:6 with lines dt 2 lw 1.5 lc rgb 'red'  title 'n log2(n) (theory)', \\\n"
        "     $DATA using 1:7 with lines dt 2 lw 1.5 lc rgb 'blue' title 'n^2 (theory)'\n\n");

    fprintf(gp,
        "set output 'time.png'\n"
        "set title 'Merge intervals: time vs n'\n"
        "set xlabel 'n (number of intervals)'\n"
        "set ylabel 'time (seconds)'\n"
        "set logscale xy\n"
        "plot $DATA using 1:4 with linespoints pt 7 lw 2 title 'sort-based O(n log n)', \\\n"
        "     $DATA using 1:5 with linespoints pt 5 lw 2 title 'naive repeated-pass'\n");

    fclose(gp);
    return 0;
}
