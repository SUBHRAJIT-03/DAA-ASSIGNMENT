/*
 * party_max_overlap.c
 *
 * n people attend a party; person i enters at time a_i and leaves at
 * time b_i (b_i > a_i). All 2n entry/exit times are distinct. Find
 * the time at which the most people were simultaneously present, in
 * O(n log n).
 *
 * Algorithm (sweep line):
 *   1. Build 2n events: (a_i, +1) for each entry, (b_i, -1) for each
 *      exit.
 *   2. Sort all 2n events by time.                    -- O(n log n)
 *   3. Sweep through in time order, maintaining a running count
 *      (add each event's delta); track the maximum count and the
 *      time at which it was first reached.             -- O(n)
 *   Total: O(n log n)
 *
 * This program:
 *   1. Stress-tests correctness against an O(n^2) brute-force check
 *      (for every entry time, count how many intervals contain it)
 *      across many random trials and sizes.
 *   2. Benchmarks the O(n log n) method's sort-comparisons against
 *      the O(n^2) brute force as n grows, writing a self-contained
 *      GNUPLOT script "plot.gnu" (data embedded inline -- no
 *      .csv/.dat/.py file).
 *
 * Compile:  gcc -O2 -o party_max_overlap party_max_overlap.c -lm
 * Run:      ./party_max_overlap
 * Plot:     gnuplot plot.gnu   (produces comparisons.png and time.png)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

typedef struct { long long t; int delta; } Event;

static long long g_cmp; /* comparisons made while sorting events */

static int cmp_event(const void *x, const void *y) {
    g_cmp++;
    const Event *a = x, *b = y;
    if (a->t < b->t) return -1;
    if (a->t > b->t) return 1;
    return 0;
}

/* ======================================================================
 *  O(n log n) SWEEP-LINE MAX OVERLAP
 * ====================================================================== */
static int max_overlap(const long long *entry, const long long *exit, int n,
                        long long *bestTimeOut) {
    Event *ev = malloc((size_t)(2 * n) * sizeof(Event));
    for (int i = 0; i < n; i++) {
        ev[2 * i]     = (Event){entry[i], +1};
        ev[2 * i + 1] = (Event){exit[i],  -1};
    }
    qsort(ev, 2 * n, sizeof(Event), cmp_event);

    int count = 0, best = 0;
    long long bestTime = 0;
    for (int i = 0; i < 2 * n; i++) {
        count += ev[i].delta;
        if (count > best) { best = count; bestTime = ev[i].t; }
    }
    free(ev);
    *bestTimeOut = bestTime;
    return best;
}

/* ======================================================================
 *  O(n^2) BRUTE FORCE (correctness baseline): the maximum overlap can
 *  only occur at one of the n entry times, so check, for each entry
 *  time, how many intervals contain it.
 * ====================================================================== */
static int max_overlap_brute(const long long *entry, const long long *exit, int n,
                              long long *bestTimeOut, long long *cmp) {
    int best = 0;
    long long bestTime = 0;
    for (int i = 0; i < n; i++) {
        long long t = entry[i];
        int count = 0;
        for (int j = 0; j < n; j++) {
            (*cmp)++;
            if (entry[j] <= t && t < exit[j]) count++;
        }
        if (count > best) { best = count; bestTime = t; }
    }
    *bestTimeOut = bestTime;
    return best;
}

/* ======================================================================
 *  HELPERS
 * ====================================================================== */

/* Generates n intervals with all 2n endpoints distinct: draws 2n
 * distinct integers, shuffles, then pairs them up (i, i+n) as
 * (entry, exit) after ensuring entry < exit for each pair. */
static void make_intervals(int n, unsigned seed, long long *entry, long long *exit) {
    int total = 2 * n;
    long long *pool = malloc((size_t)total * sizeof(long long));
    for (int i = 0; i < total; i++) pool[i] = i; /* 0..2n-1, all distinct */
    srand(seed);
    for (int i = total - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        long long tmp = pool[i]; pool[i] = pool[j]; pool[j] = tmp;
    }
    for (int i = 0; i < n; i++) {
        long long x = pool[2 * i], y = pool[2 * i + 1];
        if (x < y) { entry[i] = x; exit[i] = y; }
        else       { entry[i] = y; exit[i] = x; }
    }
    free(pool);
}

static void run_correctness_tests(void) {
    int fails = 0, total = 0;
    for (int n = 1; n <= 300; n++) {
        for (int trial = 0; trial < 3; trial++) {
            long long *entry = malloc((size_t)n * sizeof(long long));
            long long *exit  = malloc((size_t)n * sizeof(long long));
            make_intervals(n, (unsigned)(n * 97 + trial), entry, exit);

            long long tFast, tBrute;
            int fastResult = max_overlap(entry, exit, n, &tFast);

            long long dummy = 0;
            int bruteResult = max_overlap_brute(entry, exit, n, &tBrute, &dummy);

            total++;
            if (fastResult != bruteResult) {
                fails++;
                printf("FAIL: n=%d trial=%d fast=%d brute=%d\n", n, trial, fastResult, bruteResult);
            } else {
                /* also verify the returned TIME actually achieves that overlap count */
                int actualAtFastTime = 0;
                for (int j = 0; j < n; j++)
                    if (entry[j] <= tFast && tFast < exit[j]) actualAtFastTime++;
                if (actualAtFastTime != fastResult) {
                    fails++;
                    printf("FAIL: n=%d trial=%d returned time doesn't achieve claimed overlap "
                           "(claimed=%d, actual=%d)\n", n, trial, fastResult, actualAtFastTime);
                }
            }

            free(entry); free(exit);
        }
    }
    printf("Correctness: %d/%d trials passed (n = 1..300): overlap COUNT matches brute force, "
           "and returned TIME verified to actually achieve that overlap\n", total - fails, total);
}

static double sec(clock_t a, clock_t b) { return (double)(b - a) / CLOCKS_PER_SEC; }

int main(void) {
    run_correctness_tests();

    FILE *gp = fopen("plot.gnu", "w");
    if (!gp) { perror("fopen plot.gnu"); return 1; }
    fprintf(gp, "# Auto-generated by party_max_overlap.c\n");
    fprintf(gp, "# Run with:  gnuplot plot.gnu\n");
    fprintf(gp, "set terminal pngcairo size 900,650 enhanced font 'Arial,11'\n");
    fprintf(gp, "set grid\n\n");

    int sizes[] = {50, 100, 200, 400, 800, 1600, 3200, 6400, 12800};
    int nsizes = sizeof(sizes) / sizeof(sizes[0]);

    fprintf(gp, "$DATA << EOD\n");
    fprintf(gp, "# n sweep_cmp brute_cmp sweep_time brute_time n_log2n n_squared\n");

    printf("\n%-8s %-14s %-14s %-14s %-14s\n", "n", "sweep cmp", "brute cmp", "sweep time", "brute time");

    for (int s = 0; s < nsizes; s++) {
        int n = sizes[s];
        long long *entry = malloc((size_t)n * sizeof(long long));
        long long *exit  = malloc((size_t)n * sizeof(long long));
        make_intervals(n, (unsigned)(9000000u + n), entry, exit);

        g_cmp = 0;
        long long tFast;
        clock_t t0 = clock();
        max_overlap(entry, exit, n, &tFast);
        clock_t t1 = clock();
        double timeSweep = sec(t0, t1);
        long long cmpSweep = g_cmp;

        long long cmpBrute = 0;
        long long tBrute;
        t0 = clock();
        max_overlap_brute(entry, exit, n, &tBrute, &cmpBrute);
        t1 = clock();
        double timeBrute = sec(t0, t1);

        double nlog2n = 2.0 * n * log2(2.0 * n); /* 2n events sorted */
        double nsquared = (double)n * (double)n;

        printf("%-8d %-14lld %-14lld %-14.6f %-14.6f\n", n, cmpSweep, cmpBrute, timeSweep, timeBrute);
        fprintf(gp, "%d %lld %lld %.9f %.9f %.2f %.2f\n",
                n, cmpSweep, cmpBrute, timeSweep, timeBrute, nlog2n, nsquared);

        free(entry); free(exit);
    }
    fprintf(gp, "EOD\n\n");

    fprintf(gp,
        "set output 'comparisons.png'\n"
        "set title 'Max overlap: O(n log n) sweep line vs O(n^2) brute force'\n"
        "set xlabel 'n (number of people)'\n"
        "set ylabel 'comparisons'\n"
        "set logscale xy\n"
        "plot $DATA using 1:2 with linespoints pt 7 lw 2 title 'sweep line (measured)', \\\n"
        "     $DATA using 1:3 with linespoints pt 5 lw 2 title 'brute force (measured)', \\\n"
        "     $DATA using 1:6 with lines dt 2 lw 1.5 lc rgb 'red'  title '2n log2(2n) (theory)', \\\n"
        "     $DATA using 1:7 with lines dt 2 lw 1.5 lc rgb 'blue' title 'n^2 (theory)'\n\n");

    fprintf(gp,
        "set output 'time.png'\n"
        "set title 'Max overlap: time vs n'\n"
        "set xlabel 'n (number of people)'\n"
        "set ylabel 'time (seconds)'\n"
        "set logscale xy\n"
        "plot $DATA using 1:4 with linespoints pt 7 lw 2 title 'sweep line O(n log n)', \\\n"
        "     $DATA using 1:5 with linespoints pt 5 lw 2 title 'brute force O(n^2)'\n");

    fclose(gp);
    printf("\nWrote plot.gnu -- run 'gnuplot plot.gnu' to generate comparisons.png and time.png\n");
    return 0;
}
