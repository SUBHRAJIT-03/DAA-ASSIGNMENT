#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_N    20
#define KEY_LEN  16 
#define INF   1e18

double computeOBST(double p[], double q[], int n,
                   double e[][MAX_N + 2], double w[][MAX_N + 2],
                   int root[][MAX_N + 2]) {
    /* Initialise base cases (empty sub-trees) */
    for (int i = 0; i <= n; i++) {
        e[i][i]    = q[i];
        w[i][i]    = q[i];
        root[i][i] = 0;
    }

    /* l = chain length (number of keys in subtree) */
    for (int l = 1; l <= n; l++) {
        for (int i = 0; i <= n - l; i++) {
            int j = i + l;

            e[i][j]    = INF;
            w[i][j]    = w[i][j - 1] + p[j] + q[j];

            /* Try each key k (1-indexed) as root of keys [i+1..j] */
            for (int r = i + 1; r <= j; r++) {
                double cost = e[i][r - 1] + e[r][j] + w[i][j];
                if (cost < e[i][j]) {
                    e[i][j]    = cost;
                    root[i][j] = r;
                }
            }
        }
    }

    return e[0][n];
}

void printTree(int root[][MAX_N + 2], int i, int j,
               char keys[][KEY_LEN], int depth, int isLeft, int parentKey) {
    if (j < i) return;
    if (i == j) return;   /* dummy key (leaf gap) */

    int r = root[i][j];
    if (r == 0) return;

    /* Indentation */
    for (int d = 0; d < depth; d++) printf("    ");
    if (depth == 0)
        printf("Root: %s\n", keys[r]);
    else
        printf("%s child of %s: %s\n",
               isLeft ? "Left" : "Right", keys[parentKey], keys[r]);

    printTree(root, i, r - 1, keys, depth + 1, 1, r);   /* left  subtree */
    printTree(root, r, j,     keys, depth + 1, 0, r);   /* right subtree */
}


void normalise(double p[], int np, double q[], int nq) {
    double total = 0.0;
    for (int i = 1; i <= np; i++) total += p[i];
    for (int i = 0; i <= nq; i++) total += q[i];
    if (total == 0.0) { fprintf(stderr, "All-zero probabilities.\n"); return; }
    for (int i = 1; i <= np; i++) p[i] /= total;
    for (int i = 0; i <= nq; i++) q[i] /= total;
}

void getManualInput(int *n, double p[], double q[], char keys[][KEY_LEN]) {
    printf("\n Manual Input \n");
    printf("Enter number of keys (1-%d): ", MAX_N);
    scanf("%d", n);
    if (*n < 1 || *n > MAX_N) { printf("Clamping.\n"); *n = (*n < 1) ? 1 : MAX_N; }

    printf("Enter key names (up to 7 chars each):\n");
    for (int i = 1; i <= *n; i++) {
        printf("  k[%d]: ", i);
        scanf("%15s", keys[i]);
    }

    printf("Enter search probabilities p[1..%d]:\n", *n);
    for (int i = 1; i <= *n; i++) {
        printf("  p[%d]: ", i); scanf("%lf", &p[i]);
    }

    printf("Enter dummy key probabilities q[0..%d]:\n", *n);
    for (int i = 0; i <= *n; i++) {
        printf("  q[%d]: ", i); scanf("%lf", &q[i]);
    }

    normalise(p, *n, q, *n);
}

void getRandomInput(int *n, double p[], double q[], char keys[][KEY_LEN]) {
    srand((unsigned)time(NULL));

    *n = (rand() % 4) + 3;   /* 3 to 6 keys */

    printf("\n Randomised Input \n");
    printf("Number of keys: %d\n", *n);

    for (int i = 1; i <= *n; i++) {
        snprintf(keys[i], KEY_LEN, "k%d", i);
        p[i] = (double)(rand() % 10 + 1);   /* 1..10 (unnormalised) */
    }
    for (int i = 0; i <= *n; i++) {
        q[i] = (double)(rand() % 5 + 1);    /* 1..5 (unnormalised) */
    }

    normalise(p, *n, q, *n);

    printf("Keys and probabilities (normalised):\n");
    for (int i = 1; i <= *n; i++)
        printf("  p[%s] = %.4f\n", keys[i], p[i]);
    printf("Dummy key probabilities:\n");
    for (int i = 0; i <= *n; i++)
        printf("  q[d%d] = %.4f\n", i, q[i]);
}

void displayTables(double e[][MAX_N + 2], double w[][MAX_N + 2],
                   int root[][MAX_N + 2], int n) {
    (void)w;   /* w computed but display of e[] suffices */
    printf("\ne[i][j] – Expected search cost:\n");
    printf("     ");
    for (int j = 0; j <= n; j++) printf("  j=%d  ", j);
    printf("\n");
    for (int i = 0; i <= n; i++) {
        printf("i=%d  ", i);
        for (int j = 0; j <= n; j++) {
            if (j < i) printf("    ");
            else       printf(" %5.3f ", e[i][j]);
        }
        printf("\n");
    }

    printf("\nroot[i][j] – Optimal root key index:\n");
    printf("     ");
    for (int j = 1; j <= n; j++) printf("  j=%d ", j);
    printf("\n");
    for (int i = 0; i < n; i++) {
        printf("i=%d  ", i);
        for (int j = 1; j <= n; j++) {
            if (j <= i) printf("   ");
            else        printf("   %2d ", root[i][j]);
        }
        printf("\n");
    }
}

int main(void) {
    int    n;
    double p[MAX_N + 1];          /* p[1..n] */
    double q[MAX_N + 1];          /* q[0..n] */
    char   keys[MAX_N + 1][KEY_LEN];  /* key names (1-indexed) */
    int    choice;

    double e[MAX_N + 2][MAX_N + 2];
    double w[MAX_N + 2][MAX_N + 2];
    int    root[MAX_N + 2][MAX_N + 2];

    printf("Select input method:\n");
    printf("  1. Manual input\n");
    printf("  2. Randomised data\n");
    printf("Choice: ");
    scanf("%d", &choice);

    if (choice == 1)
        getManualInput(&n, p, q, keys);
    else
        getRandomInput(&n, p, q, keys);

    double minCost = computeOBST(p, q, n, e, w, root);


    printf("\n Results \n");
    printf("Minimum expected search cost: %.4f\n", minCost);
    printf("\nOptimal BST structure:\n");
    printTree(root, 0, n, keys, 0, 0, 0);

    displayTables(e, w, root, n);

    return 0;
}
