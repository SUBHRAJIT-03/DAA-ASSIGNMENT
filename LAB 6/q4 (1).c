#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void reverseSegment(int arr[], int left, int right) {
    while (left < right) {
        int temp = arr[left];
        arr[left] = arr[right];
        arr[right] = temp;
        left++;
        right--;
    }
}

void sortByReversal(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        int pos = -1;
        for (int j = i; j < n; j++) {
            if (arr[j] == i + 1) {
                pos = j;
                break;
            }
        }

        if (pos != -1 && pos != i) {
            reverseSegment(arr, i, pos);
            printf("Reverse segment [%d, %d]\n", i, pos);
        }
    }
}

void printArray(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main() {
    srand((unsigned)time(NULL));
    int mode = 0;
    printf("Choose input mode:\n1) Default permutation\n2) Manual input\n3) Random permutation\nSelect (1-3): ");
    if (scanf("%d", &mode) != 1) mode = 1;

    int n;
    int *p;

    if (mode == 1) {
        int tmp[] = {3,1,4,2,5};
        n = sizeof(tmp)/sizeof(tmp[0]);
        p = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) p[i] = tmp[i];
    } else if (mode == 2) {
        printf("Enter n (size of permutation): "); if (scanf("%d", &n) != 1) return 0;
        p = malloc(n * sizeof(int));
        printf("Enter %d values (permutation of 1..n):\n", n);
        for (int i = 0; i < n; i++) scanf("%d", &p[i]);
    } else {
        printf("Enter n (size of permutation to generate): "); if (scanf("%d", &n) != 1) return 0;
        p = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) p[i] = i + 1;
        /* Fisher-Yates shuffle */
        for (int i = n - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            int tmp = p[i]; p[i] = p[j]; p[j] = tmp;
        }
    }

    printf("Permutation before sort: ");
    printArray(p, n);

    sortByReversal(p, n);

    printf("Sorted permutation: ");
    printArray(p, n);
    free(p);
    return 0;
}
