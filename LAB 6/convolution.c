#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <time.h>

typedef double complex cplx;

#define PI 3.14159265358979323846

/* =====================================================================
   Recursive (divide-and-conquer) Cooley-Tukey FFT -- O(N log N)
   Requires N to be a power of two.

   invert == 0 : forward DFT
   invert == 1 : inverse DFT (each recursive level divides its outputs
                 by 2, so after log2(N) levels the total scaling is
                 exactly 1/N -- no separate final division is needed)
   ===================================================================== */
static void fft(cplx *a, int n, int invert) {
    if (n == 1) return; /* base case: a single point is its own transform */

    /* Divide: split into even-indexed and odd-indexed halves -- O(n) */
    cplx *a0 = malloc((size_t)(n / 2) * sizeof(cplx));
    cplx *a1 = malloc((size_t)(n / 2) * sizeof(cplx));
    for (int i = 0; i < n / 2; ++i) {
        a0[i] = a[2 * i];
        a1[i] = a[2 * i + 1];
    }

    /* Conquer: recurse on both halves -- 2T(n/2) */
    fft(a0, n / 2, invert);
    fft(a1, n / 2, invert);

    /* Combine: butterfly step using the n-th roots of unity -- O(n) */
    double angle = 2.0 * PI / n * (invert ? -1.0 : 1.0);
    cplx w = 1.0;
    cplx wn = cexp(I * angle);

    for (int i = 0; i < n / 2; ++i) {
        cplx t = w * a1[i];
        a[i] = a0[i] + t;
        a[i + n / 2] = a0[i] - t;
        if (invert) {
            a[i] /= 2.0;
            a[i + n / 2] /= 2.0;
        }
        w *= wn;
    }

    free(a0);
    free(a1);
}

static int next_pow2(int x) {
    int n = 1;
    while (n < x) n <<= 1;
    return n;
}

/* =====================================================================
   Convolution via FFT -- O(n log n)
   1. Pad A and B with zeros up to N = next power of two >= m+n-1.
   2. Evaluate both at the N-th roots of unity (forward FFT).
   3. Multiply the evaluations pointwise -- this is convolution turned
      into simple multiplication, by the convolution theorem.
   4. Interpolate back to coefficient form (inverse FFT).
   The real parts are rounded to the nearest integer since A and B
   hold integers, so the true result is an integer too (up to
   floating-point noise from the FFT).
   ===================================================================== */
static double *convolve_fft(const int *A, int m, const int *B, int n, int *out_size) {
    int result_size = m + n - 1;
    int N = next_pow2(result_size);

    cplx *fa = calloc((size_t)N, sizeof(cplx));
    cplx *fb = calloc((size_t)N, sizeof(cplx));

    for (int i = 0; i < m; ++i) fa[i] = A[i];
    for (int i = 0; i < n; ++i) fb[i] = B[i];

    fft(fa, N, 0);
    fft(fb, N, 0);

    for (int i = 0; i < N; ++i) fa[i] *= fb[i];

    fft(fa, N, 1);

    double *result = malloc((size_t)result_size * sizeof(double));
    for (int i = 0; i < result_size; ++i) result[i] = round(creal(fa[i]));

    free(fa);
    free(fb);
    *out_size = result_size;
    return result;
}

/* =====================================================================
   Naive convolution -- O(m*n)
   Directly implements C[k] = sum_j A[j]*B[k-j]. Used only to check
   the FFT-based result above; this is the "obvious" algorithm the
   O(n log n) approach is meant to beat.
   ===================================================================== */
static long long *convolve_naive(const int *A, int m, const int *B, int n, int *out_size) {
    int result_size = m + n - 1;
    long long *C = calloc((size_t)result_size, sizeof(long long));

    for (int j = 0; j < m; ++j) {
        for (int i = 0; i < n; ++i) {
            C[j + i] += (long long)A[j] * (long long)B[i];
        }
    }

    *out_size = result_size;
    return C;
}

/* =====================================================================
   Helpers: filling and printing vectors
   ===================================================================== */
static void fill_random(int *v, int len) {
    for (int i = 0; i < len; ++i) v[i] = rand() % 19 - 9; /* integers in [-9, 9] */
}

static void fill_user_input(int *v, int len, const char *label) {
    for (int i = 0; i < len; ++i) {
        printf("Enter %s[%d]: ", label, i);
        scanf("%d", &v[i]);
    }
}

static void print_int_vector(const int *v, int len) {
    printf("[ ");
    for (int i = 0; i < len; ++i) printf("%d ", v[i]);
    printf("]\n");
}

static void print_double_vector(const double *v, int len) {
    printf("[ ");
    for (int i = 0; i < len; ++i) printf("%.0f ", v[i]);
    printf("]\n");
}

static void print_ll_vector(const long long *v, int len) {
    printf("[ ");
    for (int i = 0; i < len; ++i) printf("%lld ", v[i]);
    printf("]\n");
}

/* =====================================================================
   main
   ===================================================================== */
int main(void) {
    int m, n, choice;
    srand((unsigned int)time(NULL));

    printf("Enter length of A (m): ");
    scanf("%d", &m);
    printf("Enter length of B (n): ");
    scanf("%d", &n);

    if (m <= 0 || n <= 0) {
        printf("Invalid lengths.\n");
        return 0;
    }

    if (m > n) {
        printf("Note: the algorithm assumes n >= m, so A and B (and their\n"
               "      lengths) are being swapped to satisfy that.\n");
        int tmp = m; m = n; n = tmp;
    }

    int *A = malloc((size_t)m * sizeof(int));
    int *B = malloc((size_t)n * sizeof(int));

    printf("Choose data input method:\n");
    printf("1. Random data\n");
    printf("2. User input\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    if (choice == 1) {
        fill_random(A, m);
        fill_random(B, n);
    } else {
        fill_user_input(A, m, "A");
        fill_user_input(B, n, "B");
    }

    printf("\nVector A (length %d): ", m);
    print_int_vector(A, m);
    printf("Vector B (length %d): ", n);
    print_int_vector(B, n);

    int result_size;

    clock_t t0 = clock();
    double *c_fft = convolve_fft(A, m, B, n, &result_size);
    clock_t t1 = clock();
    long long *c_naive = convolve_naive(A, m, B, n, &result_size);
    clock_t t2 = clock();

    printf("\nC = A * B via O(n log n) FFT (divide & conquer):\n");
    print_double_vector(c_fft, result_size);

    printf("C = A * B via O(m*n) naive convolution (for validation):\n");
    print_ll_vector(c_naive, result_size);

    int matches = 1;
    for (int i = 0; i < result_size; ++i) {
        if (llabs((long long)c_fft[i] - c_naive[i]) != 0) {
            matches = 0;
            break;
        }
    }

    printf("\nResults match: %s\n", matches ? "YES" : "NO");
    printf("FFT convolution time   : %.6f s\n", (double)(t1 - t0) / CLOCKS_PER_SEC);
    printf("Naive convolution time : %.6f s\n", (double)(t2 - t1) / CLOCKS_PER_SEC);

    free(A);
    free(B);
    free(c_fft);
    free(c_naive);
    return 0;
}
