#include <stdio.h>

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

    int n, k;

    printf("Enter number of elements: ");
    scanf("%d", &n);

    int arr[n];

    printf("Enter elements:\n");

    for (int i = 0; i < n; i++)
        scanf("%d", &arr[i]);

    printf("Enter K: ");
    scanf("%d", &k);

    if (k < 1 || k > n) {
        printf("Invalid K\n");
        return 0;
    }

    int result = quickSelect(arr, 0, n - 1, k - 1);

    printf("%dth smallest element = %d\n", k, result);

    return 0;
}