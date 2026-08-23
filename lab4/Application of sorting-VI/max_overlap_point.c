/*
 * max_overlap_point.c
 *
 * Given n closed intervals [l_i, r_i] on a line (endpoints count as
 * being in the interval; endpoints are NOT assumed distinct), find a
 * point contained in the largest number of intervals, in O(n log n).
 *
 * Example: S = {(10,40),(20,60),(50,90),(15,70)}. p=50 lies in
 * (20,60), (50,90), and (15,70) -- three intervals, the maximum.
 *
 * Algorithm (sweep line, with the crucial tie-break):
 *   1. Build 2n events: (l_i, START) and (r_i, END).
 *   2. Sort by coordinate; AT EQUAL COORDINATES, all STARTs must be
 *      processed before ENDs (see below for why).                -- O(n log n)
 *   3. Sweep: +1 on START, -1 on END; track the running count's
 *      maximum and the coordinate where it occurs.                 -- O(n)
 *   Total: O(n log n)
 *
 * WHY THE TIE-BREAK MATTERS: if interval A ends at x and interval B
 * starts at x, point x belongs to BOTH (closed intervals). Processing
 * B's start before A's end means the running count briefly reflects
 * both being active exactly at x -- correctly capturing that shared
 * coverage. Processing A's end first would remove A from the count
 * before B is even added, silently undercounting the overlap at x by 1.
 *
 * This program:
 *   1. Stress-tests correctness against an O(n^2) brute force (the
 *      maximum can only occur at one of the 2n endpoint coordinates,
 *      so checking coverage at each of those directly is a valid,
 *      independent ground truth) -- including test cases with
 *      DELIBERATE shared endpoints to specifically exercise the
 *      tie-breaking logic, plus the worked example from the problem
 *      statement.
 *   2. Benchmarks the O(n log n) method's sort-comparisons against
 *      the O(n^2) brute force as n grows, writing a self-contained
 *      GNUPLOT script "plot.gnu"
 *
 * Compile:  gcc -O2 -o max_overlap_point max_overlap_point.c -lm
 * Run:      ./max_overlap_point
 * Plot:     gnuplot plot.gnu   (produces comparisons.png and time.png)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

typedef struct { long long l, r; } Interval;
typedef struct { long long coord; int type; } Event; /* type: 0=START, 1=END */

static long long g_cmp;
static volatile long g_sink; /* prevents the optimizer from eliminating
                                 calls whose result would otherwise be
                                 discarded (see prior projects) */

static int cmp_event(const void *p, const void *q) {
    g_cmp++;
    const Event *a = p, *b = q;
    if (a->coord != b->coord) return (a->coord < b->coord) ? -1 : 1;
    return a->type - b->type; /* START(0) before END(1) at equal coordinates */
}

/* ======================================================================
 *  O(n log n) SWEEP LINE
 * ====================================================================== */
static int max_overlap_point(const Interval *in, int n, long long *bestPointOut) {
    Event *ev = malloc((size_t)(2 * n) * sizeof(Event));
    for (int i = 0; i < n; i++) {
        ev[2 * i]     = (Event){in[i].l, 0};
        ev[2 * i + 1] = (Event){in[i].r, 1};
    }
    qsort(ev, 2 * n, sizeof(Event), cmp_event);

    int count = 0, best = 0;
    long long bestPoint = 0;
    for (int i = 0; i < 2 * n; i++) {
        if (ev[i].type == 0) count++; else count--;
        if (count > best) { best = count; bestPoint = ev[i].coord; }
    }
    free(ev);
    *bestPointOut = bestPoint;
    return best;
}

/* ======================================================================
 *  O(n^2) BRUTE FORCE (ground truth): the maximum can only occur at
 *  one of the 2n endpoint coordinates (coverage is constant between
 *  consecutive endpoints), so check coverage directly at each.
 * ====================================================================== */
static int max_overlap_point_brute(const Interval *in, int n, long long *bestPointOut) {
    int best = 0;
    long long bestPoint = 0;
    for (int i = 0; i < n; i++) {
        long long candidates[2] = {in[i].l, in[i].r};
        for (int c = 0; c < 2; c++) {
            long long p = candidates[c];
            int count = 0;
            for (int j = 0; j < n; j++)
                if (in[j].l <= p && p <= in[j].r) count++;
            if (count > best) { best = count; bestPoint = p; }
        }
    }
    *bestPointOut = bestPoint;
    return best;
}

/* ======================================================================
 *  HELPERS
 * ====================================================================== */
static void make_random_intervals(int n, int maxCoord, unsigned seed, Interval *out) {
    srand(seed);
    for (int i = 0; i < n; i++) {
        int a = rand() % (maxCoord + 1);
        int b = rand() % (maxCoord + 1);
        out[i].l = (a < b) ? a : b;
        out[i].r = (a < b) ? b : a;
    }
}

/* Intervals with DELIBERATE shared endpoints, to specifically stress
 * the start-before-end tie-breaking logic. */
static void make_tied_intervals(int n, unsigned seed, Interval *out) {
    srand(seed);
    long long cursor = 0;
    for (int i = 0; i < n; i++) {
        int len = 1 + rand() % 5;
        out[i].l = cursor;
        out[i].r = cursor + len;
        cursor = out[i].r; /* next interval STARTS exactly where this one ENDS */
    }
    /* shuffle so input order doesn't trivially match the chain order */
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Interval t = out[i]; out[i] = out[j]; out[j] = t;
    }
}

static void run_correctness_tests(void) {
    int fails = 0, total = 0;

    /* Worked example from the problem statement */
    {
        Interval ex[] = {{10,40},{20,60},{50,90},{15,70}};
        long long p;
        int best = max_overlap_point(ex, 4, &p);
        total++;
        if (best != 3) { fails++; printf("FAIL: worked example gave count=%d, expected 3\n", best); }
        /* also verify the returned point genuinely achieves count 3 */
        int actual = 0;
        for (int j = 0; j < 4; j++) if (ex[j].l <= p && p <= ex[j].r) actual++;
        if (actual != best) { fails++; printf("FAIL: worked example point %lld achieves %d, not %d\n", p, actual, best); }
    }

    for (int n = 1; n <= 200; n++) {
        for (int trial = 0; trial < 4; trial++) {
            Interval *in = malloc((size_t)n * sizeof(Interval));
            /* alternate between generic random intervals and
             * deliberately-tied ones across trials */
            if (trial % 2 == 0) make_random_intervals(n, 50, (unsigned)(n * 97 + trial), in);
            else                make_tied_intervals(n, (unsigned)(n * 97 + trial), in);

            long long pFast, pBrute;
            int fastResult = max_overlap_point(in, n, &pFast);
            int bruteResult = max_overlap_point_brute(in, n, &pBrute);

            total++;
            if (fastResult != bruteResult) {
                fails++;
                printf("FAIL: n=%d trial=%d fast=%d brute=%d\n", n, trial, fastResult, bruteResult);
            } else {
                int actual = 0;
                for (int j = 0; j < n; j++) if (in[j].l <= pFast && pFast <= in[j].r) actual++;
                if (actual != fastResult) {
                    fails++;
                    printf("FAIL: n=%d trial=%d returned point doesn't achieve claimed count "
                           "(claimed=%d, actual=%d)\n", n, trial, fastResult, actual);
                }
            }
            free(in);
        }
    }
    printf("Correctness: %d/%d trials passed (worked example + n=1..200, mixing random and "
           "deliberately-tied-endpoint inputs; also verified returned point achieves its claimed count)\n",
           total - fails, total);
}

static double sec(clock_t a, clock_t b) { return (double)(b - a) / CLOCKS_PER_SEC; }

int main(void) {
    run_correctness_tests();

    FILE *gp = fopen("plot.gnu", "w");
    if (!gp) { perror("fopen plot.gnu"); return 1; }
    fprintf(gp, "# Auto-generated by max_overlap_point.c\n");
    fprintf(gp, "# Run with:  gnuplot plot.gnu\n");
    fprintf(gp, "set terminal pngcairo size 900,650 enhanced font 'Arial,11'\n");
    fprintf(gp, "set grid\n\n");

    int sizes[] = {50, 100, 200, 400, 800, 1600, 3200, 6400, 12800};
    int nsizes = sizeof(sizes) / sizeof(sizes[0]);

    fprintf(gp, "$DATA << EOD\n");
    fprintf(gp, "# n sweep_cmp brute_ops sweep_time brute_time n_log2n n_squared\n");

    printf("\n%-8s %-14s %-14s %-14s %-14s\n", "n", "sweep cmp", "brute ops", "sweep time", "brute time");

    for (int s = 0; s < nsizes; s++) {
        int n = sizes[s];
        Interval *in = malloc((size_t)n * sizeof(Interval));
        make_random_intervals(n, n * 4, (unsigned)(9000000u + n), in);

        g_cmp = 0;
        long long pFast;
        clock_t t0 = clock();
        max_overlap_point(in, n, &pFast);
        clock_t t1 = clock();
        double timeSweep = sec(t0, t1);
        long long cmpSweep = g_cmp;

        /* brute force op count = 2n candidate points * n interval checks = 2n^2 */
        long long opsBrute = 2LL * n * n;
        long long pBruteDummy;
        t0 = clock();
        int bruteBest = max_overlap_point_brute(in, n, &pBruteDummy);
        t1 = clock();
        g_sink ^= bruteBest ^ (long)pBruteDummy;
        double timeBrute = sec(t0, t1);

        double nlog2n = 2.0 * n * log2(2.0 * n);
        double nsquared = 2.0 * (double)n * (double)n;

        printf("%-8d %-14lld %-14lld %-14.6f %-14.6f\n", n, cmpSweep, opsBrute, timeSweep, timeBrute);
        fprintf(gp, "%d %lld %lld %.9f %.9f %.2f %.2f\n",
                n, cmpSweep, opsBrute, timeSweep, timeBrute, nlog2n, nsquared);

        free(in);
    }
    fprintf(gp, "EOD\n\n");

    fprintf(gp,
        "set output 'comparisons.png'\n"
        "set title 'Max overlap point: O(n log n) sweep line vs O(n^2) brute force'\n"
        "set xlabel 'n (number of intervals)'\n"
        "set ylabel 'comparisons / operations'\n"
        "set logscale xy\n"
        "plot $DATA using 1:2 with linespoints pt 7 lw 2 title 'sweep line (measured)', \\\n"
        "     $DATA using 1:3 with linespoints pt 5 lw 2 title 'brute force (measured)', \\\n"
        "     $DATA using 1:6 with lines dt 2 lw 1.5 lc rgb 'red'  title '2n log2(2n) (theory)', \\\n"
        "     $DATA using 1:7 with lines dt 2 lw 1.5 lc rgb 'blue' title '2n^2 (theory)'\n\n");

    fprintf(gp,
        "set output 'time.png'\n"
        "set title 'Max overlap point: time vs n'\n"
        "set xlabel 'n (number of intervals)'\n"
        "set ylabel 'time (seconds)'\n"
        "set logscale xy\n"
        "plot $DATA using 1:4 with linespoints pt 7 lw 2 title 'sweep line O(n log n)', \\\n"
        "     $DATA using 1:5 with linespoints pt 5 lw 2 title 'brute force O(n^2)'\n");

    fclose(gp);
    printf("\nWrote plot.gnu -- run 'gnuplot plot.gnu' to generate comparisons.png and time.png\n");
    return 0;
}
