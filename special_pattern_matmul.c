/*
 * Two n x n matrices (n = 2^k) with the recursive block structure
 *     M = [ M1  M2 ]
 *         [ M2  M1 ]
 * where each block M1, M2 recursively has the same structure down to
 * single scalars. This program multiplies two such matrices in
 * Theta(n^2) using a Karatsuba-style divide-and-conquer trick,
 * instead of the naive Theta(n^3).
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

static long long g_mults;  /* scalar multiplications (base case only) */
static long long g_adds;   /* scalar additions/subtractions (combine steps) */

/* ======================================================================
 *  GENERATE a random matrix with the required recursive block
 *  structure: M = [[M1,M2],[M2,M1]], each block built the same way
 *  down to n=1 (a random scalar).
 * ====================================================================== */
/* Simple portable PRNG (avoids POSIX-only rand_r, which MinGW lacks) */
static unsigned lcg_next(unsigned *state) {
    *state = (*state) * 1103515245u + 12345u;
    return (*state >> 16) & 0x7fff;
}

static void gen_special_rec(int *M, int ld, int n, unsigned *seed) {
    if (n == 1) { M[0] = (int)(lcg_next(seed) % 21) - 10; return; }
    int h = n / 2;
    int *M1 = M, *M2 = M + h;
    gen_special_rec(M1, ld, h, seed);      /* top-left block M1 */
    gen_special_rec(M2, ld, h, seed);      /* top-right block M2 */
    /* mirror into bottom-left (=M2) and bottom-right (=M1) */
    for (int i = 0; i < h; i++)
        for (int j = 0; j < h; j++) {
            M[(h + i) * ld + j]     = M2[i * ld + j]; /* bottom-left = M2 */
            M[(h + i) * ld + h + j] = M1[i * ld + j]; /* bottom-right = M1 */
        }
}
static int *gen_special(int n, unsigned seed) {
    int *M = malloc((size_t)n * n * sizeof(int));
    gen_special_rec(M, n, n, &seed);
    return M;
}

/* Verify a full n x n matrix genuinely has the required recursive
 * block structure (used as a sanity check on the ALGORITHM'S OUTPUT,
 * confirming the closure property the whole approach relies on). */
static int is_special_rec(const int *M, int ld, int n) {
    if (n == 1) return 1;
    int h = n / 2;
    for (int i = 0; i < h; i++)
        for (int j = 0; j < h; j++) {
            if (M[(h + i) * ld + j]     != M[i * ld + h + j]) return 0; /* bottom-left == top-right */
            if (M[(h + i) * ld + h + j] != M[i * ld + j])     return 0; /* bottom-right == top-left */
        }
    return is_special_rec(M, ld, h) && is_special_rec(M + h, ld, h);
}
static int is_special(const int *M, int n) { return is_special_rec(M, n, n); }

/* ======================================================================
 *  BLOCK HELPERS (pointers into larger buffers, ldX = leading dim)
 * ====================================================================== */
static void add_block(const int *A, int ldA, const int *B, int ldB, int *C, int ldC, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) { g_adds++; C[i * ldC + j] = A[i * ldA + j] + B[i * ldB + j]; }
}
static void sub_block(const int *A, int ldA, const int *B, int ldB, int *C, int ldC, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) { g_adds++; C[i * ldC + j] = A[i * ldA + j] - B[i * ldB + j]; }
}

/* ======================================================================
 *  O(n^2) DIVIDE AND CONQUER MULTIPLY FOR SPECIAL-PATTERN MATRICES
 *  (only needs A1,A2,B1,B2 -- the TOP half of each input -- since the
 *  bottom half is guaranteed identical to the top by the structure)
 * ====================================================================== */
static void multiply_special(const int *A, int ldA, const int *B, int ldB,
                              int *C, int ldC, int n) {
    if (n == 1) { g_mults++; C[0] = A[0] * B[0]; return; }

    int h = n / 2;
    const int *A1 = A,     *A2 = A + h;
    const int *B1 = B,     *B2 = B + h;

    size_t bytes = (size_t)h * h * sizeof(int);
    int *SA = malloc(bytes), *SB = malloc(bytes);
    int *P1 = malloc(bytes), *P2 = malloc(bytes), *P3 = malloc(bytes);
    int *C11 = malloc(bytes), *C12 = malloc(bytes);

    add_block(A1, ldA, A2, ldA, SA, h, h);
    add_block(B1, ldB, B2, ldB, SB, h, h);

    multiply_special(A1, ldA, B1, ldB, P1, h, h);
    multiply_special(A2, ldA, B2, ldB, P2, h, h);
    multiply_special(SA, h,  SB, h,  P3, h, h);

    add_block(P1, h, P2, h, C11, h, h);
    sub_block(P3, h, P1, h, C12, h, h);
    sub_block(C12, h, P2, h, C12, h, h);

    /* write all four quadrants of C (top-left/bottom-right = C11,
     * top-right/bottom-left = C12) -- exploiting the guaranteed
     * output symmetry instead of recomputing it */
    for (int i = 0; i < h; i++)
        for (int j = 0; j < h; j++) {
            C[i * ldC + j]             = C11[i * h + j];
            C[i * ldC + h + j]         = C12[i * h + j];
            C[(h + i) * ldC + j]       = C12[i * h + j];
            C[(h + i) * ldC + h + j]   = C11[i * h + j];
        }

    free(SA); free(SB); free(P1); free(P2); free(P3); free(C11); free(C12);
}

/* ======================================================================
 *  NAIVE FULL MULTIPLY (baseline / correctness reference)
 * ====================================================================== */
static void naive_multiply(const int *A, const int *B, int *C, int n, long long *mults) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            long sum = 0;
            for (int k = 0; k < n; k++) { (*mults)++; sum += (long)A[i * n + k] * B[k * n + j]; }
            C[i * n + j] = (int)sum;
        }
}

/* ======================================================================
 *  CORRECTNESS TESTS
 * ====================================================================== */
static void run_correctness_tests(void) {
    int sizes[] = {1, 2, 4, 8, 16, 32, 64, 128};
    int ns = sizeof(sizes) / sizeof(sizes[0]);
    int fails = 0, total = 0;

    for (int s = 0; s < ns; s++) {
        int n = sizes[s];
        for (int trial = 0; trial < 3; trial++) {
            int *A = gen_special(n, (unsigned)(n * 131 + trial * 7 + 1));
            int *B = gen_special(n, (unsigned)(n * 131 + trial * 7 + 2));

            if (!is_special(A, n) || !is_special(B, n)) {
                printf("BUG: generated matrix does not have the required structure (n=%d)\n", n);
            }

            int *C_dc = malloc((size_t)n * n * sizeof(int));
            int *C_naive = malloc((size_t)n * n * sizeof(int));
            long long dummyMults = 0;

            multiply_special(A, n, B, n, C_dc, n, n);
            naive_multiply(A, B, C_naive, n, &dummyMults);

            total++;
            int ok = 1;
            for (int i = 0; i < n * n; i++) if (C_dc[i] != C_naive[i]) { ok = 0; break; }
            if (!ok) { fails++; printf("FAIL: n=%d trial=%d mismatch vs naive\n", n, trial); }

            if (!is_special(C_dc, n)) {
                printf("FAIL: n=%d trial=%d output does NOT have the required structure\n", n, trial);
                fails++;
            }

            free(A); free(B); free(C_dc); free(C_naive);
        }
    }
    printf("Correctness: %d/%d trials passed (n = 1..128; also verified output structure)\n",
           total - fails, total);
}

/* ======================================================================
 *  MAIN
 * ====================================================================== */
int main(void) {
    run_correctness_tests();

    FILE *gp = fopen("plot.gnu", "w");
    if (!gp) { perror("fopen plot.gnu"); return 1; }
    fprintf(gp, "# Auto-generated by special_pattern_matmul.c\n");
    fprintf(gp, "# Run with:  gnuplot plot.gnu\n");
    fprintf(gp, "set terminal pngcairo size 900,650 enhanced font 'Arial,11'\n");
    fprintf(gp, "set grid\n");
    fprintf(gp, "set logscale xy\n\n");

    int sizes[] = {2, 4, 8, 16, 32, 64, 128, 256, 512};
    int ns = sizeof(sizes) / sizeof(sizes[0]);

    fprintf(gp, "$DATA << EOD\n");
    fprintf(gp, "# n dc_mults dc_adds dc_total naive_mults n_log2_3 n_squared n_cubed\n");

    printf("\n%-8s %-14s %-14s %-14s %-16s\n", "n", "D&C mults", "D&C adds", "D&C total", "naive mults(n^3)");

    for (int s = 0; s < ns; s++) {
        int n = sizes[s];
        int *A = gen_special(n, (unsigned)(9000000u + n));
        int *B = gen_special(n, (unsigned)(8000000u + n));
        int *C = malloc((size_t)n * n * sizeof(int));

        g_mults = 0; g_adds = 0;
        multiply_special(A, n, B, n, C, n, n);
        long long dcMults = g_mults, dcAdds = g_adds, dcTotal = g_mults + g_adds;
        long long naiveMults = (long long)n * n * n;

        double n_log2_3 = pow((double)n, log2(3.0));
        double n_squared = (double)n * n;
        double n_cubed = (double)n * n * n;

        printf("%-8d %-14lld %-14lld %-14lld %-16lld\n", n, dcMults, dcAdds, dcTotal, naiveMults);
        fprintf(gp, "%d %lld %lld %lld %lld %.2f %.2f %.2f\n",
                n, dcMults, dcAdds, dcTotal, naiveMults, n_log2_3, n_squared, n_cubed);

        free(A); free(B); free(C);
    }
    fprintf(gp, "EOD\n\n");

    /* --- Plot 1: multiplication count alone: grows as n^log2(3), NOT n^2 --- */
    fprintf(gp,
        "set output 'mults.png'\n"
        "set title 'Special-pattern D&C: scalar MULTIPLICATIONS alone grow as n^log2(3)'\n"
        "set xlabel 'n (matrix size)'\n"
        "set ylabel 'count'\n"
        "plot $DATA using 1:2 with linespoints pt 7 lw 2 title 'D&C multiplications (measured)', \\\n"
        "     $DATA using 1:6 with lines dt 2 lw 1.5 lc rgb 'red' title 'n^log2(3) (theory)', \\\n"
        "     $DATA using 1:7 with lines dt 3 lw 1.5 lc rgb 'gray40' title 'n^2 (reference)'\n\n");

    /* --- Plot 2: total ops (mults+adds) vs naive: Theta(n^2) vs Theta(n^3) --- */
    fprintf(gp,
        "set output 'totalops.png'\n"
        "set title 'Special-pattern D&C (Theta(n^2)) vs Naive (Theta(n^3))'\n"
        "set xlabel 'n (matrix size)'\n"
        "set ylabel 'total elementary operations'\n"
        "plot $DATA using 1:4 with linespoints pt 7 lw 2 title 'D&C total ops (mults+adds, measured)', \\\n"
        "     $DATA using 1:5 with linespoints pt 5 lw 2 title 'Naive multiplications (measured, = n^3)', \\\n"
        "     $DATA using 1:7 with lines dt 2 lw 1.5 lc rgb 'red'  title 'n^2 (theory, D&C)', \\\n"
        "     $DATA using 1:8 with lines dt 2 lw 1.5 lc rgb 'blue' title 'n^3 (theory, naive)'\n");

    fclose(gp);
    return 0;
}
