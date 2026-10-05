#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_N 1000

int computeLIS(int A[], int n, int dp[], int parent[]) {
    int maxLen   = 1;
    int bestEnd  = 0;   /* index where the longest LIS ends */

    for (int i = 0; i < n; i++) {
        dp[i]     = 1;   /* LIS of length 1 ending at i */
        parent[i] = -1;

        for (int j = 0; j < i; j++) {
            if (A[j] < A[i] && dp[j] + 1 > dp[i]) {
                dp[i]     = dp[j] + 1;
                parent[i] = j;
            }
        }

        if (dp[i] > maxLen) {
            maxLen  = dp[i];
            bestEnd = i;
        }
    }

    parent[n] = bestEnd;

    return maxLen;
}

void printLIS(int A[], int n, int parent[]) {
    int bestEnd = parent[n];   /* stored by computeLIS() */

    /* Collect path */
    int path[MAX_N];
    int pathLen = 0;
    int idx = bestEnd;

    while (idx != -1) {
        path[pathLen++] = A[idx];
        idx = parent[idx];
    }

    /* Reverse for correct order */
    printf("LIS elements: [ ");
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
    printf("Enter %d integers: ", *n);
    for (int i = 0; i < *n; i++) scanf("%d", &A[i]);
}

void getRandomInput(int A[], int *n) {
    srand((unsigned)time(NULL));

    *n = (rand() % 16) + 5;   /* 5 to 20 elements */
    printf("\n Randomised Input \n");
    printf("Array (n = %d): ", *n);
    for (int i = 0; i < *n; i++) {
        A[i] = (rand() % 101) - 50;   /* -50 to 50 */
        printf("%d ", A[i]);
    }
    printf("\n");
}

void displayDP(int A[], int dp[], int n) {
    printf("\nDP Table (index → LIS length ending here):\n");
    printf("%-8s %-8s %-8s\n", "Index", "Value", "dp[i]");
    printf("-\n");
    for (int i = 0; i < n; i++)
        printf("%-8d %-8d %-8d\n", i, A[i], dp[i]);
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

    int *dp     = (int *)malloc(n * sizeof(int));
    int *parent = (int *)malloc((n + 1) * sizeof(int));

    if (!dp || !parent) {
        fprintf(stderr, "Memory allocation failed.\n");
        free(dp); free(parent);
        return 1;
    }

    int lisLen = computeLIS(A, n, dp, parent);

    printf("\n Results \n");
    printf("Array     : ");
    for (int i = 0; i < n; i++) printf("%d ", A[i]);
    printf("\nLIS Length: %d\n", lisLen);
    printLIS(A, n, parent);

    displayDP(A, dp, n);


    free(dp);
    free(parent);
    return 0;
}
