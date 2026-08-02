#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    int n, heads = 0;
    double probability;

    printf("Enter the number of tosses: ");
    scanf("%d", &n);

    srand(time(NULL));

    // Fair coin simulation
    for (int i = 0; i < n; i++) {
        if (rand() % 2 == 0)
            heads++;
    }

    probability = (double)heads / n;

    printf("\n--- Fair Coin ---\n");
    printf("Heads = %d\n", heads);
    printf("Tails = %d\n", n - heads);
    printf("Estimated Probability of Head = %.4f\n", probability);

    // Biased coin simulation (70% chance of Head)
    heads = 0;

    for (int i = 0; i < n; i++) {
        double r = (double)rand() / RAND_MAX;

        if (r < 0.7)
            heads++;
    }

    probability = (double)heads / n;

    printf("\n--- Biased Coin (70%% Head) ---\n");
    printf("Heads = %d\n", heads);
    printf("Tails = %d\n", n - heads);
    printf("Estimated Probability of Head = %.4f\n", probability);

    return 0;
}