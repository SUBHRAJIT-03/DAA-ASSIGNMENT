#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LEN 100


#define MATCH    0   /* characters matched: move diagonal */
#define INSERT   1   /* came from dp[i][j-1] */
#define DELETE   2   /* came from dp[i-1][j] */

int computeLCS(const char X[], const char Y[], int m, int n,
               int dp[][MAX_LEN + 1], int dir[][MAX_LEN + 1]) {
    /* Base cases */
    for (int i = 0; i <= m; i++) dp[i][0] = 0;
    for (int j = 0; j <= n; j++) dp[0][j] = 0;

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (X[i - 1] == Y[j - 1]) {
                dp[i][j]  = dp[i - 1][j - 1] + 1;
                dir[i][j] = MATCH;
            } else if (dp[i - 1][j] >= dp[i][j - 1]) {
                dp[i][j]  = dp[i - 1][j];
                dir[i][j] = DELETE;
            } else {
                dp[i][j]  = dp[i][j - 1];
                dir[i][j] = INSERT;
            }
        }
    }

    return dp[m][n];
}

void traceback(const char X[], int m, int n,
               int dir[][MAX_LEN + 1], char lcs[]) {
    int idx = 0;
    int i = m, j = n;

    while (i > 0 && j > 0) {
        if (dir[i][j] == MATCH) {
            lcs[idx++] = X[i - 1];
            i--; j--;
        } else if (dir[i][j] == DELETE) {
            i--;
        } else {
            j--;
        }
    }
    lcs[idx] = '\0';

    /* The string was built in reverse – flip it */
    int left = 0, right = idx - 1;
    while (left < right) {
        char tmp  = lcs[left];
        lcs[left] = lcs[right];
        lcs[right] = tmp;
        left++; right--;
    }
}


void getManualInput(char X[], char Y[]) {
    printf("\n Manual Input \n");
    printf("Enter sequence X (max %d chars): ", MAX_LEN);
    scanf("%100s", X);
    printf("Enter sequence Y (max %d chars): ", MAX_LEN);
    scanf("%100s", Y);
}

void getRandomInput(char X[], char Y[]) {
    srand((unsigned)time(NULL));

    int lenX = (rand() % 11) + 5;
    int lenY = (rand() % 11) + 5;

    for (int i = 0; i < lenX; i++) X[i] = 'a' + (rand() % 8); /* a-h */
    X[lenX] = '\0';

    for (int i = 0; i < lenY; i++) Y[i] = 'a' + (rand() % 8);
    Y[lenY] = '\0';

    printf("\n Randomised Input \n");
    printf("Sequence X (%d chars): %s\n", lenX, X);
    printf("Sequence Y (%d chars): %s\n", lenY, Y);
}

void displayDP(const char X[], const char Y[], int m, int n,
               int dp[][MAX_LEN + 1]) {
    printf("\nDP Table (rows = X, cols = Y):\n");

    /* Header row */
    printf("     ");
    printf("  [ε]");
    for (int j = 0; j < n; j++) printf("  [%c]", Y[j]);
    printf("\n");

    /* Data rows */
    for (int i = 0; i <= m; i++) {
        if (i == 0) printf(" [ε] ");
        else        printf(" [%c] ", X[i - 1]);

        for (int j = 0; j <= n; j++) {
            printf("  %3d", dp[i][j]);
        }
        printf("\n");
    }
}

int main(void) {
    char X[MAX_LEN + 1], Y[MAX_LEN + 1];
    int choice;

    printf("Select input method:\n");
    printf("  1. Manual input\n");
    printf("  2. Randomised data\n");
    printf("Choice: ");
    scanf("%d", &choice);

    if (choice == 1)
        getManualInput(X, Y);
    else
        getRandomInput(X, Y);

    int m = (int)strlen(X);
    int n = (int)strlen(Y);

    /* Allocate DP and direction tables */
    int (*dp)[MAX_LEN + 1]  = malloc((MAX_LEN + 1) * (MAX_LEN + 1) * sizeof(int));
    int (*dir)[MAX_LEN + 1] = malloc((MAX_LEN + 1) * (MAX_LEN + 1) * sizeof(int));
    char lcs[MAX_LEN + 1];

    if (!dp || !dir) {
        fprintf(stderr, "Memory allocation failed.\n");
        free(dp); free(dir);
        return 1;
    }

    int lcsLen = computeLCS(X, Y, m, n, dp, dir);
    traceback(X, m, n, dir, lcs);

    /* ── Output ── */
    printf("\n Results \n");
    printf("Sequence X : \"%s\"  (length %d)\n", X, m);
    printf("Sequence Y : \"%s\"  (length %d)\n", Y, n);
    printf("LCS Length : %d\n", lcsLen);
    printf("LCS String : \"%s\"\n", lcs);

    /* Only print table if sequences are short enough to display cleanly */
    if (m <= 15 && n <= 15)
        displayDP(X, Y, m, n, dp);
    else
        printf("(DP table too large to display — sequences exceed 15 chars)\n");

    free(dp);
    free(dir);
    return 0;
}
