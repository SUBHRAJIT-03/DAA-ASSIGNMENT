#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_N 1000

long long computeMSIS(int A[], int n, long long dp[], int parent[]) {
    long long maxSum  = 0;
    int       bestEnd = 0;

    for (int i = 0; i < n; i++) {
        dp[i]     = A[i];   /* subsequence containing only A[i] */
        parent[i] = -1;

        for (int j = 0; j < i; j++) {
            if (A[j] < A[i] && dp[j] + A[i] > dp[i]) {
                dp[i]     = dp[j] + A[i];
                parent[i] = j;
            }
        }

        if (dp[i] > maxSum) {
            maxSum  = dp[i];
            bestEnd = i;
        }
    }

    parent[n] = bestEnd;   /* store best-ending index for traceback */
    return maxSum;
}

void printMSIS(int A[], int n, int parent[]) {
    int bestEnd = parent[n];

    int path[MAX_N];
    int pathLen = 0;
    int idx = bestEnd;

    while (idx != -1) {
        path[pathLen++] = A[idx];
        idx = parent[idx];
    }

    printf("Subsequence (max sum): [ ");
    for (int i = pathLen - 1; i >= 0; i--)
        printf("%d ", path[i]);
    printf("]\n");
}


void getManualInput(int A[], int *n) {
    printf("\n Manual Input \n");
    printf("Enter array size (1-%d): ", MAX_N);
    scanf("%d", n);
    if (*n < 1 || *n > MAX_N) {
        printf("Invalid size. Clamping.\n");
        *n = (*n < 1) ? 1 : MAX_N;
    }
    printf("Enter %d POSITIVE integers: ", *n);
    for (int i = 0; i < *n; i++) {
        scanf("%d", &A[i]);
        if (A[i] <= 0) { printf("(Warning: non-positive value %d used)\n", A[i]); }
    }
}

void getRandomInput(int A[], int *n) {
    srand((unsigned)time(NULL));

    *n = (rand() % 16) + 5;   /* 5 to 20 */
    printf("\n Randomised Input \n");
    printf("Array (n = %d): ", *n);
    for (int i = 0; i < *n; i++) {
        A[i] = (rand() % 50) + 1;   /* 1 to 50 (positive) */
        printf("%d ", A[i]);
    }
    printf("\n");
}

void displayDP(int A[], long long dp[], int n) {
    printf("\nDP Table (index → max sum of increasing subseq ending here):\n");
    printf("%-8s %-8s %-12s\n", "Index", "Value", "dp[i] (sum)");
    printf("\n");
    for (int i = 0; i < n; i++)
        printf("%-8d %-8d %-12lld\n", i, A[i], dp[i]);
}

int main(void) {
    int A[MAX_N];
    int n, choice;

    printf("Select input method:\n");
    printf("  1. Manual input\n");
    printf("  2. Randomised data\n");
    printf("Choice: ");
    scanf("%d", &choice);

    if (choice == 1)
        getManualInput(A, &n);
    else
        getRandomInput(A, &n);

    long long *dp = (long long *)malloc(n * sizeof(long long));
    int *parent   = (int *)malloc((n + 1) * sizeof(int));

    if (!dp || !parent) {
        fprintf(stderr, "Memory allocation failed.\n");
        free(dp); free(parent);
        return 1;
    }

    long long result = computeMSIS(A, n, dp, parent);

    printf("\n Results \n");
    printf("Array      : ");
    for (int i = 0; i < n; i++) printf("%d ", A[i]);
    printf("\nMax Sum    : %lld\n", result);
    printMSIS(A, n, parent);

    displayDP(A, dp, n);


    free(dp);
    free(parent);
    return 0;
}
