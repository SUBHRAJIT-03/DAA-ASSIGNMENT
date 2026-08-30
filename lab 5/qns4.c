#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void heapify(int arr[], int n, int i) {

    int largest = i;

    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && arr[left] > arr[largest])
        largest = left;

    if (right < n && arr[right] > arr[largest])
        largest = right;

    if (largest != i) {

        swap(&arr[i], &arr[largest]);

        heapify(arr, n, largest);
    }
}

void heapSort(int arr[], int n) {

    /* Build Max Heap */

    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i);

    /* Extract elements */

    for (int i = n - 1; i > 0; i--) {

        swap(&arr[0], &arr[i]);

        heapify(arr, i, 0);
    }
}

int main() {

    int n;

    printf("Enter number of elements: ");
    scanf("%d", &n);

    int arr[n];

    FILE *fp;

    srand(time(NULL));

    /* Generate random elements and store in file */

    fp = fopen("random_heap.txt", "w");

    if (fp == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    printf("\nRandom elements:\n");

    for (int i = 0; i < n; i++) {

        arr[i] = rand() % 100;

        fprintf(fp, "%d ", arr[i]);

        printf("%d ", arr[i]);
    }

    fclose(fp);

    /* Read elements from file */

    fp = fopen("random_heap.txt", "r");

    if (fp == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    for (int i = 0; i < n; i++)
        fscanf(fp, "%d", &arr[i]);

    fclose(fp);

    /* Heap Sort */

    heapSort(arr, n);

    printf("\n\nSorted elements:\n");

    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);

    printf("\n");

    return 0;
}