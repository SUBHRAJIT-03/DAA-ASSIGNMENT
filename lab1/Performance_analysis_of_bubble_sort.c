/*(i) Bubble Sort with Early Termination

This version stops if no swaps occur during a pass, meaning the array is already sorted.*/ 

#include <stdio.h>

void bubbleSortEarly(int arr[], int n) {
    int i, j, temp;
    int swapped;
    int comparisons = 0;

    for (i = 0; i < n - 1; i++) {
        swapped = 0;

        for (j = 0; j < n - i - 1; j++) {
            comparisons++;

            if (arr[j] > arr[j + 1]) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                swapped = 1;
            }
        }

        if (swapped == 0)
            break;
    }

    printf("Comparisons (Early Termination): %d\n", comparisons);
}

int main() {
    int arr[] = {5, 1, 4, 2, 8};
    int n = sizeof(arr) / sizeof(arr[0]);

    bubbleSortEarly(arr, n);

    printf("Sorted Array: ");
    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);

    return 0;
}
/*(ii) Bubble Sort without Early Termination

This version always performs all (n−1) passes.*/

#include <stdio.h>

void bubbleSortNormal(int arr[], int n) {
    int i, j, temp;
    int comparisons = 0;

    for (i = 0; i < n - 1; i++) {

        for (j = 0; j < n - i - 1; j++) {
            comparisons++;

            if (arr[j] > arr[j + 1]) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }

    printf("Comparisons (Normal Bubble Sort): %d\n", comparisons);
}

int main() {
    int arr[] = {5, 1, 4, 2, 8};
    int n = sizeof(arr) / sizeof(arr[0]);

    bubbleSortNormal(arr, n);

    printf("Sorted Array: ");
    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);

    return 0;
}
/*Comparison
Feature	                      Early Termination	Normal Bubble Sort
Stops if already sorted	                Yes	    No
Best-case Time	                        O(n)	O(n²)
Average-case Time	                    O(n²)	O(n²)
Worst-case Time                     	O(n²)	O(n²)
Efficient on sorted/nearly sorted data	Yes	    No
*/
