/*
 * DAA Lab-02, Q2: Merge sort (2-way, divide into halves) vs modified
 * merge sort (3-way, divide into thirds).
 * Both are O(n log n); this program times both for growing n and writes
 * results_q2.csv so the order of growth can be plotted.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

static double now_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
}

/* ---------------- Standard 2-way merge sort ---------------- */
void merge2(int *a, int lo, int mid, int hi, int *tmp) {
    int i = lo, j = mid, k = lo;
    while (i < mid && j < hi) tmp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    while (i < mid) tmp[k++] = a[i++];
    while (j < hi)  tmp[k++] = a[j++];
    memcpy(a + lo, tmp + lo, (hi - lo) * sizeof(int));
}
void mergesort2(int *a, int lo, int hi, int *tmp) {
    if (hi - lo <= 1) return;
    int mid = lo + (hi - lo) / 2;
    mergesort2(a, lo, mid, tmp);
    mergesort2(a, mid, hi, tmp);
    merge2(a, lo, mid, hi, tmp);
}

/* ---------------- Modified 3-way merge sort ---------------- */
void merge3(int *a, int lo, int m1, int m2, int hi, int *tmp) {
    int i = lo, j = m1, k = m2, t = lo;
    while (i < m1 && j < m2 && k < hi) {
        if (a[i] <= a[j] && a[i] <= a[k]) tmp[t++] = a[i++];
        else if (a[j] <= a[i] && a[j] <= a[k]) tmp[t++] = a[j++];
        else tmp[t++] = a[k++];
    }
    while (i < m1 && j < m2) tmp[t++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    while (j < m2 && k < hi) tmp[t++] = (a[j] <= a[k]) ? a[j++] : a[k++];
    while (i < m1 && k < hi) tmp[t++] = (a[i] <= a[k]) ? a[i++] : a[k++];
    while (i < m1) tmp[t++] = a[i++];
    while (j < m2) tmp[t++] = a[j++];
    while (k < hi) tmp[t++] = a[k++];
    memcpy(a + lo, tmp + lo, (hi - lo) * sizeof(int));
}
void mergesort3(int *a, int lo, int hi, int *tmp) {
    if (hi - lo <= 1) return;
    int len = hi - lo;
    int m1 = lo + len / 3;
    int m2 = lo + 2 * len / 3;
    if (m1 == lo) m1++;         /* guard tiny ranges */
    if (m2 <= m1) m2 = m1 + 1;
    if (m2 >= hi) m2 = hi - 1;
    mergesort3(a, lo, m1, tmp);
    mergesort3(a, m1, m2, tmp);
    mergesort3(a, m2, hi, tmp);
    merge3(a, lo, m1, m2, hi, tmp);
}

int* random_array(int n) {
    int *a = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) a[i] = rand();
    return a;
}
int is_sorted(int *a, int n) {
    for (int i = 1; i < n; i++) if (a[i - 1] > a[i]) return 0;
    return 1;
}

int main(void) {
    srand(7);
    int sizes[] = {1000, 5000, 10000, 20000, 40000, 80000, 160000, 320000, 640000};
    int nsizes = sizeof(sizes) / sizeof(sizes[0]);
    int REPEATS = 5;

    FILE *out = fopen("results_q2.csv", "w");
    fprintf(out, "n,mergesort2_us,mergesort3_us\n");

    for (int s = 0; s < nsizes; s++) {
        int n = sizes[s];
        double total2 = 0, total3 = 0;
        for (int r = 0; r < REPEATS; r++) {
            int *a = random_array(n);
            int *b = malloc(n * sizeof(int));
            int *tmp = malloc(n * sizeof(int));
            memcpy(b, a, n * sizeof(int));

            double t = now_us();
            mergesort2(a, 0, n, tmp);
            total2 += now_us() - t;
            if (!is_sorted(a, n)) { fprintf(stderr, "mergesort2 BUG at n=%d\n", n); }

            t = now_us();
            mergesort3(b, 0, n, tmp);
            total3 += now_us() - t;
            if (!is_sorted(b, n)) { fprintf(stderr, "mergesort3 BUG at n=%d\n", n); }

            free(a); free(b); free(tmp);
        }
        fprintf(out, "%d,%.3f,%.3f\n", n, total2 / REPEATS, total3 / REPEATS);
        printf("n=%7d  mergesort2=%.1fus  mergesort3=%.1fus\n", n, total2 / REPEATS, total3 / REPEATS);
    }
    fclose(out);
    printf("Wrote results_q2.csv\n");
    return 0;
}
