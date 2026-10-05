#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LEN 100

#define OP_MATCH   0   /* no change needed (chars equal) */
#define OP_SUBST   1   /* substitution */
#define OP_DELETE  2   /* delete from A (move up in table) */
#define OP_INSERT  3   /* insert into A (move left in table)*/

int computeEditDistance(const char A[], const char B[], int m, int n,
                        int dp[][MAX_LEN + 1], int dir[][MAX_LEN + 1]) {
    /* Base cases */
    for (int i = 0; i <= m; i++) { dp[i][0] = i; dir[i][0] = OP_DELETE; }
    for (int j = 0; j <= n; j++) { dp[0][j] = j; dir[0][j] = OP_INSERT; }
    dir[0][0] = OP_MATCH;

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (A[i - 1] == B[j - 1]) {
                dp[i][j]  = dp[i - 1][j - 1];
                dir[i][j] = OP_MATCH;
            } else {
                int sub = dp[i - 1][j - 1] + 1;
                int del = dp[i - 1][j]     + 1;
                int ins = dp[i][j - 1]     + 1;

                if (sub <= del && sub <= ins) {
                    dp[i][j]  = sub;
                    dir[i][j] = OP_SUBST;
                } else if (del <= ins) {
                    dp[i][j]  = del;
                    dir[i][j] = OP_DELETE;
                } else {
                    dp[i][j]  = ins;
                    dir[i][j] = OP_INSERT;
                }
            }
        }
    }

    return dp[m][n];
}

void traceback(const char A[], const char B[], int m, int n,
               int dir[][MAX_LEN + 1]) {
    char opDetails[2 * MAX_LEN + 5][80];
    int  opCount = 0;

    int i = m, j = n;
    while (i > 0 || j > 0) {
        switch (dir[i][j]) {
            case OP_MATCH:
                snprintf(opDetails[opCount], 80,
                         "KEEP    A[%d]='%c'", i - 1, A[i - 1]);
                i--; j--;
                break;
            case OP_SUBST:
                snprintf(opDetails[opCount], 80,
                         "REPLACE A[%d]='%c' -> '%c'", i - 1, A[i - 1], B[j - 1]);
                i--; j--;
                break;
            case OP_DELETE:
                snprintf(opDetails[opCount], 80,
                         "DELETE  A[%d]='%c'", i - 1, A[i - 1]);
                i--;
                break;
            case OP_INSERT:
                snprintf(opDetails[opCount], 80,
                         "INSERT  '%c' before position %d", B[j - 1], i);
                j--;
                break;
            default:
                i--; j--;
                break;
        }
        opCount++;
    }

    printf("\nOperation Traceback (forward order):\n");
    printf("%-6s %-s\n", "Step", "Operation");
    for (int k = opCount - 1; k >= 0; k--)
        printf("%-6d %s\n", opCount - k, opDetails[k]);
}

void displayDP(const char A[], const char B[], int m, int n,
               int dp[][MAX_LEN + 1]) {
    printf("\nEdit Distance DP Table:\n");

    /* Column header */
    printf("     ");
    printf("  [ε]");
    for (int j = 0; j < n; j++) printf("  [%c]", B[j]);
    printf("\n");

    for (int i = 0; i <= m; i++) {
        if (i == 0) printf(" [ε] ");
        else        printf(" [%c] ", A[i - 1]);

        for (int j = 0; j <= n; j++)
            printf("  %3d", dp[i][j]);
        printf("\n");
    }
}

void getManualInput(char A[], char B[]) {
    printf("\n Manual Input \n");
    printf("Enter string A (max %d chars): ", MAX_LEN);
    scanf("%100s", A);
    printf("Enter string B (max %d chars): ", MAX_LEN);
    scanf("%100s", B);
}

void getRandomInput(char A[], char B[]) {
    srand((unsigned)time(NULL));

    int lenA = (rand() % 7) + 4;
    int lenB = (rand() % 7) + 4;

    for (int i = 0; i < lenA; i++) A[i] = 'a' + (rand() % 6);
    A[lenA] = '\0';

    for (int i = 0; i < lenB; i++) B[i] = 'a' + (rand() % 6);
    B[lenB] = '\0';

    printf("\n Randomised Input \n");
    printf("String A (%d chars): %s\n", lenA, A);
    printf("String B (%d chars): %s\n", lenB, B);
}

int main(void) {
    char A[MAX_LEN + 1], B[MAX_LEN + 1];
    int choice;

    printf("Select input method:\n");
    printf("  1. Manual input\n");
    printf("  2. Randomised data\n");
    printf("Choice: ");
    scanf("%d", &choice);

    if (choice == 1)
        getManualInput(A, B);
    else
        getRandomInput(A, B);

    int m = (int)strlen(A);
    int n = (int)strlen(B);

    int (*dp)[MAX_LEN + 1]  = malloc((MAX_LEN + 1) * (MAX_LEN + 1) * sizeof(int));
    int (*dir)[MAX_LEN + 1] = malloc((MAX_LEN + 1) * (MAX_LEN + 1) * sizeof(int));

    if (!dp || !dir) {
        fprintf(stderr, "Memory allocation failed.\n");
        free(dp); free(dir);
        return 1;
    }

    int dist = computeEditDistance(A, B, m, n, dp, dir);

    printf("\n Results \n");
    printf("String A   : \"%s\"  (length %d)\n", A, m);
    printf("String B   : \"%s\"  (length %d)\n", B, n);
    printf("Edit Distance: %d\n", dist);

    traceback(A, B, m, n, dir);

    if (m <= 12 && n <= 12)
        displayDP(A, B, m, n, dp);
    else
        printf("(DP table too large to display — strings exceed 12 chars)\n");

    free(dp);
    free(dir);
    return 0;
}
