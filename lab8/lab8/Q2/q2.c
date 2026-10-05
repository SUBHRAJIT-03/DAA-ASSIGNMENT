#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_COINS  20
#define MAX_AMOUNT 1000


long long countWays(int coins[], int n, int V, long long dp[]) {
    dp[0] = 1;          /* one way to make amount 0: use no coins */

    for (int v = 1; v <= V; v++)
        dp[v] = 0;

    /* Outer loop: coins (ensures each combination counted once) */
    for (int i = 0; i < n; i++) {
        /* Inner loop: amounts */
        for (int v = coins[i]; v <= V; v++) {
            dp[v] += dp[v - coins[i]];
        }
    }

    return dp[V];
}

void getManualInput(int coins[], int *n, int *V) {
    printf("\n Manual Input \n");
    printf("Enter number of coin denominations (1-%d): ", MAX_COINS);
    scanf("%d", n);

    if (*n < 1 || *n > MAX_COINS) {
        printf("Invalid count. Clamping to valid range.\n");
        *n = (*n < 1) ? 1 : MAX_COINS;
    }

    printf("Enter %d distinct denomination(s): ", *n);
    for (int i = 0; i < *n; i++)
        scanf("%d", &coins[i]);

    printf("Enter target amount V (1-%d): ", MAX_AMOUNT);
    scanf("%d", V);
}


void getRandomInput(int coins[], int *n, int *V) {
    srand((unsigned)time(NULL));

    *n = (rand() % 4) + 2;   /* 2 to 5 denominations */
    *V = (rand() % 41) + 10; /* target: 10 to 50     */

    /* Generate distinct values in range [1..15] */
    int pool[15];
    for (int i = 0; i < 15; i++) pool[i] = i + 1;

    /* Fisher-Yates shuffle of pool */
    for (int i = 14; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = pool[i]; pool[i] = pool[j]; pool[j] = tmp;
    }

    printf("\n Randomised Input \n");
    printf("Generated %d distinct denominations for target V = %d\n", *n, *V);
    printf("Coins: ");
    for (int i = 0; i < *n; i++) {
        coins[i] = pool[i];
        printf("%d ", coins[i]);
    }
    printf("\n");
}

void displayDP(long long dp[], int V) {
    printf("\nDP Table (amount → number of combinations):\n");
    printf("%-8s %-15s\n", "Amount", "Combinations");
    printf("\n");
    int limit = (V < 20) ? V : 20;
    for (int v = 0; v <= limit; v++)
        printf("%-8d %-15lld\n", v, dp[v]);
    if (V > 20) printf("... (showing first 21 rows of %d)\n", V + 1);
}

int main(void) {
    int coins[MAX_COINS];
    int n, V;
    int choice;

    printf("Select input method:\n");
    printf("  1. Manual input\n");
    printf("  2. Randomised data\n");
    printf("Choice: ");
    scanf("%d", &choice);

    if (choice == 1)
        getManualInput(coins, &n, &V);
    else
        getRandomInput(coins, &n, &V);

    long long *dp = (long long *)malloc((V + 1) * sizeof(long long));
    if (!dp) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }

    long long result = countWays(coins, n, V, dp);

    /* ── Output ── */
    printf("\n Results \n");
    printf("Denominations: ");
    for (int i = 0; i < n; i++) printf("%d ", coins[i]);
    printf("\nTarget amount V = %d\n", V);
    printf("Total distinct combinations: %lld\n", result);

    displayDP(dp, V);

    free(dp);
    return 0;
}
