#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int year;
    int type; // +1 birth -1 death
} Event;

// Comparator for qsort
// Sort primarily by year (ascending).
// If years match, sort by type (ascending) so -1 (death) comes before +1 (birth).
int cmp_events(const void *a, const void *b) {
    Event *ea = (Event *)a;
    Event *eb = (Event *)b;
    if (ea->year != eb->year) {
        return ea->year - eb->year;
    }
    return ea->type - eb->type;
}

void solution(int *birth, int *death, int n) {
    int num_events = n * 2;
    Event *events = malloc(num_events * sizeof(Event));
    if (!events) {
        printf("Memory allocation failed");
        return;
    }

    // Populate events
    for (int i = 0; i < n; i++) {
        events[2 * i].year = birth[i];
        events[2 * i].type = 1;     // Birth increases count
        events[2 * i + 1].year = death[i];
        events[2 * i + 1].type = -1; // Death decreases count
    }

    // Sort events
    qsort(events, num_events, sizeof(Event), cmp_events);

    int max_alive = 0;
    int current_alive = 0;
    int best_year = events[0].year;

    // Sweep line through the sorted events
    for (int i = 0; i < num_events; i++) {
        current_alive += events[i].type;
        if (current_alive > max_alive) {
            max_alive = current_alive;
            best_year = events[i].year;
        }
    }

    printf("\nRESULT: The best year to be alive was %d with %d scientists alive\n", best_year, max_alive);
    free(events);
}

void userInput(int **birth, int **death, int *n) {
    printf("Enter number of persons m: ");
    if (scanf("%d", n) != 1 || *n <= 0) {
        fprintf(stderr, "Invalid input for m.\n");
        *n = 0; // 0 means fail
    }
    
    *birth = malloc(sizeof(int) * (*n));
    *death = malloc(sizeof(int) * (*n));
    
    for (int i = 0; i < *n; ++i) {
        printf("Enter birth and death year for person %d: ", i + 1);
        if (scanf("%d %d", &(*birth)[i], &(*death)[i]) != 2) {
            printf("Invalid input for years.\n");
        }
    }
}

void randomInput(int **birth, int **death, int *n) {
    int maxm = 20, start = 1800, span = 300;
    printf("Enter number of persons to generate (default 20): ");
    
    if (scanf("%d", n) != 1 || *n <= 0) {
        *n = maxm;
    }
    
    *birth = malloc(sizeof(int) * (*n));
    *death = malloc(sizeof(int) * (*n));
    srand((unsigned)time(NULL));
    
    printf("\nGenerated Data (Birth - Death):\n");
    for (int i = 0; i < *n; ++i) {
        int b = start + (rand() % span);
        int d = b + (rand() % 60); // lived for 60yrs
        (*birth)[i] = b;
        (*death)[i] = d;
        
        // Print first 10 only
        if (i < 10) {
            printf("Person %d: %d - %d\n", i + 1, b, d);
        }
    }
    if (*n > 10) printf("... and %d more entries\n", *n - 10);
    printf("Generated %d intervals\n", *n);
}

int main() {
    int mode = 0;
    int n = 0;
    int *birth = NULL;
    int *death = NULL;

    printf("The Best Time to Be Alive\n");
    printf("Choose input mode:\n 1) Manual (enter m intervals)\n 2) Random generation\nEnter 1 or 2: ");
    
    if (scanf("%d", &mode) != 1) return 1;

    if (mode == 1)
        userInput(&birth, &death, &n);
    else
        randomInput(&birth, &death, &n);

    solution(birth, death, n);

    free(birth);
    free(death);
    return 0;
}