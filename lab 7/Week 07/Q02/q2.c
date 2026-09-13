#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

int max(int a, int b) {
    return (a > b) ? a : b;
}

int eggDropDP(int E, int F) {
    // A 2D array where dp[i][j] represents the minimum number of trials
    // needed for i eggs and j floors.
    int **dp = (int **)malloc((E + 1) * sizeof(int *));
    for (int i = 0; i <= E; i++) {
        dp[i] = (int *)malloc((F + 1) * sizeof(int));
    }

    // Base Case 1: 0 trials for 0 floors, and 1 trial for 1 floor
    for (int i = 1; i <= E; i++) {
        dp[i][0] = 0;
        dp[i][1] = 1;
    }

    // Base Case 2: We need j trials for 1 egg and j floors (linear search)
    for (int j = 1; j <= F; j++) {
        dp[1][j] = j;
    }

    // Fill the DP table using the recurrence relation
    for (int i = 2; i <= E; i++) {       // i eggs
        for (int j = 2; j <= F; j++) {   // j floors
            dp[i][j] = INT_MAX;
            
            // Try dropping from every floor x from 1 to j
            for (int x = 1; x <= j; x++) {
                // 1 drop + worst-case outcome of the drop
                int res = 1 + max(dp[i - 1][x - 1], dp[i][j - x]);
                
                if (res < dp[i][j]) {
                    dp[i][j] = res;
                }
            }
        }
    }

    // Store result before freeing memory
    int result = dp[E][F];

    // Free the dynamically allocated memory
    for (int i = 0; i <= E; i++) {
        free(dp[i]);
    }
    free(dp);

    return result;
}

void userInput(int *E, int *F) {
    printf("Super egg testing experiment\n");
    printf("Enter number of eggs E (>=1): ");
    if (scanf("%d", E) != 1) return;
    
    printf("Enter number of floors F (>=1): ");
    if (scanf("%d", F) != 1) return;

    if (*E < 1 || *F < 1) { 
        printf("E and F must be >= 1\n"); 
        return; 
    }

}

void randomInput(int *E, int *F) {
    int maxE = 5, maxF = 100;
    printf("Enter max eggs (default 5): ");
    if (scanf("%d", &maxE) != 1) maxE = 5;
    printf("Enter max floors (default 100): ");
    if (scanf("%d", &maxF) != 1) maxF = 100;
    if (maxE < 1) maxE = 5;
    if (maxF < 1) maxF = 100;
    srand((unsigned)time(NULL));
    *E = (rand() % maxE) + 1;
    *F = (rand() % maxF) + 1;
    printf("Randomly chosen E=%d, F=%d\n", *E, *F);
}

int main(void) {
    int eggs, floors;

    int mode = 0;
    printf("Choose input mode:\n 1) Manual (enter E and F)\n2) Random generation\nEnter 1 or 2: ");
    if (scanf("%d", &mode) != 1) return 1;
    if (mode == 1) userInput(&eggs, &floors);
    else randomInput(&eggs, &floors);
        
    int min_trials = eggDropDP(eggs, floors);
    
    printf("\nMinimum number of droppings guaranteed to find the highest safe floor: %d\n", min_trials);

    return 0;
}