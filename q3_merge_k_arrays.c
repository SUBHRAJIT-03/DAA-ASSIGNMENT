/*
 * DAA Lab-02, Q3: Merging k sorted arrays, each of size n.
 *
 * Method 1: sequential merging.
 *   merge(a1,a2) -> r2 (size 2n); merge(r2,a3) -> r3 (size 3n); ...
 *   Worst case: O(k^2 * n)
 *
 * Method 2: pairwise (tournament) merging.
 *   Merge k arrays in pairs -> k/2 arrays of size 2n, repeat log2(k) rounds.
 *   Worst case: O(k * n * log k)
 *
 * This program times both methods for (a) fixed n, growing k, and
 * (b) fixed k, growing n, writing two CSVs.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static double now_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
}

/* merge two sorted arrays a[0..na) and b[0..nb) into out (size na+nb) */
void merge_two(const int *a, int na, const int *b, int nb, int *out) {
    int i = 0, j = 0, k = 0;
    while (i < na && j < nb) out[k++] = (a[i] <= b[j]) ? a[i++] : b[j++];
    while (i < na) out[k++] = a[i++];
    while (j < nb) out[k++] = b[j++];
}

/* ---------------- Method 1: sequential merging ---------------- */
/* arrays: k arrays each of length n (already sorted). Returns malloc'd sorted array of size k*n. */
int* merge_sequential(int **arrays, int k, int n) {
    int *result = malloc(n * sizeof(int));
    memcpy(result, arrays[0], n * sizeof(int));
    int result_len = n;
    for (int i = 1; i < k; i++) {
        int *merged = malloc((result_len + n) * sizeof(int));
        merge_two(result, result_len, arrays[i], n, merged);
        free(result);
        result = merged;
        result_len += n;
    }
    return result;
}

/* ---------------- Method 2: pairwise / tournament merging ---------------- */
int* merge_pairwise(int **arrays, int k, int n) {
    /* copy pointers/lengths into working lists we can halve each round */
    int cur_count = k;
    int **cur = malloc(k * sizeof(int*));
    int *lens = malloc(k * sizeof(int));
    for (int i = 0; i < k; i++) {
        cur[i] = malloc(n * sizeof(int));
        memcpy(cur[i], arrays[i], n * sizeof(int));
        lens[i] = n;
    }
    while (cur_count > 1) {
        int next_count = (cur_count + 1) / 2;
        int **next = malloc(next_count * sizeof(int*));
        int *next_lens = malloc(next_count * sizeof(int));
        int idx = 0;
        for (int i = 0; i < cur_count; i += 2) {
            if (i + 1 < cur_count) {
                int nl = lens[i] + lens[i + 1];
                int *m = malloc(nl * sizeof(int));
                merge_two(cur[i], lens[i], cur[i + 1], lens[i + 1], m);
                free(cur[i]); free(cur[i + 1]);
                next[idx] = m; next_lens[idx] = nl; idx++;
            } else {
                next[idx] = cur[i]; next_lens[idx] = lens[i]; idx++;
            }
        }
        free(cur); free(lens);
        cur = next; lens = next_lens; cur_count = next_count;
    }
    int *result = cur[0];
    free(cur); free(lens);
    return result;
}

int** make_k_sorted_arrays(int k, int n) {
    int **arrays = malloc(k * sizeof(int*));
    for (int i = 0; i < k; i++) {
        arrays[i] = malloc(n * sizeof(int));
        int v = rand() % 50;
        for (int j = 0; j < n; j++) { v += rand() % 50 + 1; arrays[i][j] = v; }
    }
    return arrays;
}
void free_k_arrays(int **arrays, int k) {
    for (int i = 0; i < k; i++) free(arrays[i]);
    free(arrays);
}
int is_sorted(int *a, int n) {
    for (int i = 1; i < n; i++) if (a[i - 1] > a[i]) return 0;
    return 1;
}

int main(void) {
    srand(123);

    /* ---- Experiment A: fixed n, growing k ---- */
    {
        int n = 200;
        int ks[] = {2, 4, 8, 16, 32, 64, 128, 256};
        int nk = sizeof(ks) / sizeof(ks[0]);
        FILE *out = fopen("results_q3_vary_k.csv", "w");
        fprintf(out, "k,n,sequential_us,pairwise_us\n");
        for (int idx = 0; idx < nk; idx++) {
            int k = ks[idx];
            int **arrays = make_k_sorted_arrays(k, n);

            double t = now_us();
            int *r1 = merge_sequential(arrays, k, n);
            double t1 = now_us() - t;
            if (!is_sorted(r1, k * n)) fprintf(stderr, "seq BUG k=%d\n", k);

            t = now_us();
            int *r2 = merge_pairwise(arrays, k, n);
            double t2 = now_us() - t;
            if (!is_sorted(r2, k * n)) fprintf(stderr, "pair BUG k=%d\n", k);

            fprintf(out, "%d,%d,%.3f,%.3f\n", k, n, t1, t2);
            printf("k=%4d n=%4d  sequential=%9.1fus  pairwise=%9.1fus\n", k, n, t1, t2);

            free(r1); free(r2);
            free_k_arrays(arrays, k);
        }
        fclose(out);
        printf("Wrote results_q3_vary_k.csv\n\n");
    }

    /* ---- Experiment B: fixed k, growing n ---- */
    {
        int k = 32;
        int ns[] = {100, 200, 400, 800, 1600, 3200, 6400, 12800};
        int nn = sizeof(ns) / sizeof(ns[0]);
        FILE *out = fopen("results_q3_vary_n.csv", "w");
        fprintf(out, "k,n,sequential_us,pairwise_us\n");
        for (int idx = 0; idx < nn; idx++) {
            int n = ns[idx];
            int **arrays = make_k_sorted_arrays(k, n);

            double t = now_us();
            int *r1 = merge_sequential(arrays, k, n);
            double t1 = now_us() - t;

            t = now_us();
            int *r2 = merge_pairwise(arrays, k, n);
            double t2 = now_us() - t;

            fprintf(out, "%d,%d,%.3f,%.3f\n", k, n, t1, t2);
            printf("k=%4d n=%5d  sequential=%9.1fus  pairwise=%9.1fus\n", k, n, t1, t2);

            free(r1); free(r2);
            free_k_arrays(arrays, k);
        }
        fclose(out);
        printf("Wrote results_q3_vary_n.csv\n");
    }
    return 0;
}
