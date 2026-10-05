#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define MAX_COINS  20
#define MAX_AMOUNT 10000

int minCoins(int coins[], int n, int V, int dp[], int parent[]) {
    /* Initialise: 0 coins needed for amount 0; INF for everything else */
    dp[0]     = 0;
    parent[0] = -1;

    for (int v = 1; v <= V; v++) {
        dp[v]     = INT_MAX;   /* represents "not reachable" */
        parent[v] = -1;
    }

    /* Bottom-up fill */
    for (int v = 1; v <= V; v++) {
        for (int i = 0; i < n; i++) {
            if (coins[i] <= v && dp[v - coins[i]] != INT_MAX) {
                int candidate = dp[v - coins[i]] + 1;
                if (candidate < dp[v]) {
                    dp[v]     = candidate;
                    parent[v] = coins[i];   /* remember which coin led here */
                }
            }
        }
    }

    return (dp[V] == INT_MAX) ? -1 : dp[V];
}


void printCoinsUsed(int parent[], int V) {
    if (parent[V] == -1) {
        printf("No valid combination exists.\n");
        return;
    }
    printf("Coins used (traceback): ");
    int amt = V;
    while (amt > 0) {
        printf("%d ", parent[amt]);
        amt -= parent[amt];
    }
    printf("\n");
}

void getManualInput(int coins[], int *n, int *V) {
    printf("\n Manual Input \n");
    printf("Enter number of coin denominations (1-%d): ", MAX_COINS);
    scanf("%d", n);

    if (*n < 1 || *n > MAX_COINS) {
        printf("Invalid count. Clamping to valid range.\n");
        *n = (*n < 1) ? 1 : MAX_COINS;
    }

    printf("Enter %d denomination(s): ", *n);
    for (int i = 0; i < *n; i++) {
        scanf("%d", &coins[i]);
    }

    printf("Enter target amount V (1-%d): ", MAX_AMOUNT);
    scanf("%d", V);
}

void getRandomInput(int coins[], int *n, int *V) {
    srand((unsigned)time(NULL));

    *n = (rand() % 5) + 2;   /* 2 to 6 denominations */
    *V = (rand() % 91) + 10; /* target: 10 to 100    */

    printf("\n Randomised Input \n");
    printf("Generated %d denominations for target V = %d\n", *n, *V);

    /* Ensure denomination 1 is always present so a solution always exists */
    coins[0] = 1;
    printf("Coins: 1 ");
    for (int i = 1; i < *n; i++) {
        coins[i] = (rand() % 10) + 2;  /* 2..11 */
        printf("%d ", coins[i]);
    }
    printf("\n");
}

void displayDP(int dp[], int V) {
    printf("\nDP Table (amount → min coins):\n");
    printf("%-8s %-10s\n", "Amount", "Min Coins");
    printf("--\n");
    for (int v = 0; v <= V && v <= 20; v++) {   /* show first 21 entries */
        if (dp[v] == INT_MAX)
            printf("%-8d %-10s\n", v, "INF");
        else
            printf("%-8d %-10d\n", v, dp[v]);
    }
    if (V > 5) printf("... (showing first 21 rows of %d)\n", V + 1);
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

    /* Allocate DP and traceback tables */
    int *dp     = (int *)malloc((V + 1) * sizeof(int));
    int *parent = (int *)malloc((V + 1) * sizeof(int));

    if (!dp || !parent) {
        fprintf(stderr, "Memory allocation failed.\n");
        free(dp); free(parent);
        return 1;
    }

    int result = minCoins(coins, n, V, dp, parent);

    printf("\n Results \n");
    printf("Denominations: ");
    for (int i = 0; i < n; i++) printf("%d ", coins[i]);
    printf("\nTarget amount V = %d\n", V);

    if (result == -1) {
        printf("Result: Cannot make amount %d with given denominations.\n", V);
    } else {
        printf("Minimum coins needed: %d\n", result);
        printCoinsUsed(parent, V);
    }

    displayDP(dp, V);

    free(dp);
    free(parent);
    return 0;
}
