#include <stdio.h>
#include <stdlib.h>

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int arr[], int low, int high) {
    int pivot = arr[high];
    int i = low;

    for (int j = low; j < high; j++) {
        if (arr[j] <= pivot) {
            swap(&arr[i], &arr[j]);
            i++;
        }
    }

    swap(&arr[i], &arr[high]);

    return i;
}

int quickSelect(int arr[], int low, int high, int k) {

    while (low <= high) {

        int p = partition(arr, low, high);

        if (p == k)
            return arr[p];

        else if (k < p)
            high = p - 1;

        else
            low = p + 1;
    }

    return -1;
}

int main() {

    int n;

    printf("Enter number of elements: ");
    scanf("%d", &n);

    int arr[n];

    printf("Enter elements:\n");
    for (int i = 0; i < n; i++)
        scanf("%d", &arr[i]);

    double median;

    if (n % 2 == 1) {

        int middle = n / 2;

        median = quickSelect(arr, 0, n - 1, middle);
    }
    else {

        int middle1 = n / 2 - 1;
        int middle2 = n / 2;

        int a = quickSelect(arr, 0, n - 1, middle1);
        int b = quickSelect(arr, 0, n - 1, middle2);

        median = (a + b) / 2.0;
    }

    printf("Median = %.2f\n", median);

    return 0;
}