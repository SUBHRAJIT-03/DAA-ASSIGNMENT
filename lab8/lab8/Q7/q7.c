#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ROD 100

int rodCutting(int P[], int n, int dp[], int cut[]) {
    dp[0]  = 0;
    cut[0] = 0;

    for (int l = 1; l <= n; l++) {
        int best = -1;
        for (int k = 1; k <= l; k++) {
            int revenue = P[k] + dp[l - k];
            if (revenue > best) {
                best    = revenue;
                cut[l]  = k;   /* optimal first cut */
            }
        }
        dp[l] = best;
    }

    return dp[n];
}

void printCuts(int cut[], int n) {
    printf("Optimal cuts: [ ");
    while (n > 0) {
        printf("%d ", cut[n]);
        n -= cut[n];
    }
    printf("]\n");
}

void getManualInput(int P[], int *n) {
    printf("\n Manual Input \n");
    printf("Enter rod length n (1-%d): ", MAX_ROD);
    scanf("%d", n);
    if (*n < 1 || *n > MAX_ROD) {
        printf("Invalid length. Clamping.\n");
        *n = (*n < 1) ? 1 : MAX_ROD;
    }
    printf("Enter prices P[1..%d] (price for each integer length):\n", *n);
    for (int i = 1; i <= *n; i++) {
        printf("  P[%d] = ", i);
        scanf("%d", &P[i]);
    }
}

void getRandomInput(int P[], int *n) {
    srand((unsigned)time(NULL));

    *n = (rand() % 11) + 5;   /* 5 to 15 */

    printf("\n Randomised Input \n");
    printf("Rod length n = %d\n", *n);
    printf("Price table P[1..%d]:\n", *n);
    for (int i = 1; i <= *n; i++) {
        P[i] = i + (rand() % (2 * i + 1));   /* i .. 3i */
        printf("  P[%d] = %d\n", i, P[i]);
    }
}

void displayDP(int dp[], int cut[], int n) {
    printf("\nDP Table:\n");
    printf("%-8s %-14s %-10s\n", "Length", "Max Revenue", "First Cut");
    for (int l = 0; l <= n; l++)
        printf("%-8d %-14d %-10d\n", l, dp[l], cut[l]);
}


int main(void) {
    int P[MAX_ROD + 1]; 
    int n, choice;

    printf("Select input method:\n");
    printf("  1. Manual input\n");
    printf("  2. Randomised data\n");
    printf("Choice: ");
    scanf("%d", &choice);

    if (choice == 1)
        getManualInput(P, &n);
    else
        getRandomInput(P, &n);

    int *dp  = (int *)malloc((n + 1) * sizeof(int));
    int *cut = (int *)malloc((n + 1) * sizeof(int));

    if (!dp || !cut) {
        fprintf(stderr, "Memory allocation failed.\n");
        free(dp); free(cut);
        return 1;
    }

    int maxRevenue = rodCutting(P, n, dp, cut);

    printf("\n Results \n");
    printf("Rod length      : %d\n", n);
    printf("Maximum revenue : %d\n", maxRevenue);
    printCuts(cut, n);

    displayDP(dp, cut, n);

    free(dp);
    free(cut);
    return 0;
}
