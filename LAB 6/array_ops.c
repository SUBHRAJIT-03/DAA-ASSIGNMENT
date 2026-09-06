#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* =====================================================================
   Helpers: filling and printing the array
   ===================================================================== */

static void fill_random(int *arr, int n) {
    for (int i = 0; i < n; ++i) {
        arr[i] = rand() % 100;
    }
}

static void fill_user_input(int *arr, int n) {
    for (int i = 0; i < n; ++i) {
        printf("Enter element %d: ", i + 1);
        scanf("%d", &arr[i]);
    }
}

static void print_array(const int *arr, int n) {
    for (int i = 0; i < n; ++i) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

static void swap_int(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

/* =====================================================================
   (i) Maximum element -- O(n)
   Single linear scan, keeping the largest value seen so far.
   ===================================================================== */
static int find_max(const int *arr, int n) {
    int max = arr[0];
    for (int i = 1; i < n; ++i) {
        if (arr[i] > max) max = arr[i];
    }
    return max;
}

/* =====================================================================
   (ii) First and second largest -- O(n)
   One pass, updating two tracked values instead of scanning twice.
   ===================================================================== */
static void find_first_second_largest(const int *arr, int n, int *first, int *second) {
    if (arr[0] > arr[1]) {
        *first = arr[0];
        *second = arr[1];
    } else {
        *first = arr[1];
        *second = arr[0];
    }

    for (int i = 2; i < n; ++i) {
        if (arr[i] > *first) {
            *second = *first;
            *first = arr[i];
        } else if (arr[i] > *second) {
            *second = arr[i];
        }
    }
}

/* =====================================================================
   (iii) Mean -- O(n)
   Running sum in a single pass, divided by n.
   ===================================================================== */
static double find_mean(const int *arr, int n) {
    long long sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];
    }
    return (double)sum / n;
}

/* =====================================================================
   (iv) Median -- randomized Quickselect
   Average-case O(n), worst-case O(n^2). Reuses the same random-pivot
   partitioning idea as (ix). The array is copied first so the caller's
   original data (and its original order) is left untouched.
   ===================================================================== */
static int partition_lomuto(int *arr, int left, int right) {
    int pivot = arr[right];
    int store_index = left;

    for (int i = left; i < right; ++i) {
        if (arr[i] <= pivot) {
            swap_int(&arr[store_index], &arr[i]);
            ++store_index;
        }
    }
    swap_int(&arr[store_index], &arr[right]);
    return store_index;
}

static int quickselect(int *arr, int left, int right, int k_index) {
    while (left <= right) {
        int pivot_index = partition_lomuto(arr, left, right);
        if (pivot_index == k_index) {
            return arr[pivot_index];
        }
        if (pivot_index < k_index) {
            left = pivot_index + 1;
        } else {
            right = pivot_index - 1;
        }
    }
    return -1;
}

static double find_median(const int *arr, int n) {
    int *copy_one = malloc((size_t)n * sizeof(int));
    if (copy_one == NULL) return 0.0;
    for (int i = 0; i < n; ++i) copy_one[i] = arr[i];

    if (n % 2 == 1) {
        int middle = n / 2;
        int median = quickselect(copy_one, 0, n - 1, middle);
        free(copy_one);
        return (double)median;
    }

    int *copy_two = malloc((size_t)n * sizeof(int));
    if (copy_two == NULL) {
        free(copy_one);
        return 0.0;
    }
    for (int i = 0; i < n; ++i) copy_two[i] = arr[i];

    int left_middle = quickselect(copy_one, 0, n - 1, (n / 2) - 1);
    int right_middle = quickselect(copy_two, 0, n - 1, n / 2);

    free(copy_one);
    free(copy_two);
    return (left_middle + right_middle) / 2.0;
}

/* =====================================================================
   (v) Standard deviation -- O(n)
   Needs the mean first (O(n)), then one more pass to sum squared
   deviations (O(n)). Total remains O(n).
   ===================================================================== */
static double find_std_dev(const int *arr, int n, double mean) {
    double sum_sq = 0.0;
    for (int i = 0; i < n; ++i) {
        double diff = arr[i] - mean;
        sum_sq += diff * diff;
    }
    return sqrt(sum_sq / n);
}

/* =====================================================================
   (vi) Mode -- naive O(n^2)
   For every element, count how often it occurs in the whole array and
   keep the value with the highest count. (A hash-based frequency map
   would bring the average case down to O(n) at the cost of O(n) space.)
   ===================================================================== */
static int find_mode(const int *arr, int n) {
    int mode = arr[0];
    int max_count = 0;

    for (int i = 0; i < n; ++i) {
        int count = 0;
        for (int j = 0; j < n; ++j) {
            if (arr[j] == arr[i]) count++;
        }
        if (count > max_count) {
            max_count = count;
            mode = arr[i];
        }
    }
    return mode;
}

/* =====================================================================
   (vii) Removing all duplicates -- naive O(n^2), order-preserving
   Each candidate element is checked against everything already kept.
   Worst case (all elements distinct) this is 0+1+2+...+(n-1) = O(n^2).
   (A hash-set would bring the average case down to O(n).)
   ===================================================================== */
static int remove_duplicates(int *arr, int n) {
    int new_n = 0;
    for (int i = 0; i < n; ++i) {
        int found = 0;
        for (int j = 0; j < new_n; ++j) {
            if (arr[j] == arr[i]) {
                found = 1;
                break;
            }
        }
        if (!found) arr[new_n++] = arr[i];
    }
    return new_n;
}

/* =====================================================================
   (viii) Reversing the array -- O(n)
   Two pointers moving towards each other, swapping as they go.
   ===================================================================== */
static void reverse_array(int *arr, int n) {
    int left = 0, right = n - 1;
    while (left < right) {
        swap_int(&arr[left], &arr[right]);
        ++left;
        --right;
    }
}

/* =====================================================================
   (ix) Partition around a random pivot -- O(n)
   Rearranges the array so that every element >= pivot comes before
   every element < pivot (i.e. all elements less than the pivot end up
   after all elements greater than or equal to it). This is a Lomuto-
   style single pass with the comparison direction flipped, and it is
   exactly the building block used by quickselect in (iv).
   Returns the final index of the pivot.
   ===================================================================== */
static int partition_random_pivot(int *arr, int n) {
    int pivot_index = rand() % n;
    int pivot = arr[pivot_index];
    swap_int(&arr[pivot_index], &arr[n - 1]); /* move pivot out of the way */

    int store_index = 0;
    for (int i = 0; i < n - 1; ++i) {
        if (arr[i] >= pivot) {
            swap_int(&arr[store_index], &arr[i]);
            ++store_index;
        }
    }
    swap_int(&arr[store_index], &arr[n - 1]); /* drop pivot into its slot */
    return store_index;
}

/* =====================================================================
   main
   ===================================================================== */
int main(void) {
    int n, choice;
    srand((unsigned int)time(NULL));

    printf("Enter the size of the dataset: ");
    scanf("%d", &n);

    if (n <= 0) {
        printf("Invalid size.\n");
        return 0;
    }

    int *arr = malloc((size_t)n * sizeof(int));
    if (arr == NULL) {
        printf("Memory allocation failed.\n");
        return 0;
    }

    printf("Choose data input method:\n");
    printf("1. Random data\n");
    printf("2. User input\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    if (choice == 1) {
        fill_random(arr, n);
    } else {
        fill_user_input(arr, n);
    }

    printf("\nOriginal dataset: ");
    print_array(arr, n);

    /* (i) */
    printf("\n(i)   Maximum element                 : %d\n", find_max(arr, n));

    /* (ii) */
    if (n >= 2) {
        int first, second;
        find_first_second_largest(arr, n, &first, &second);
        printf("(ii)  First largest / second largest : %d / %d\n", first, second);
    } else {
        printf("(ii)  Need at least 2 elements for first/second largest.\n");
    }

    /* (iii) */
    double mean = find_mean(arr, n);
    printf("(iii) Mean                            : %.2f\n", mean);

    /* (iv) */
    printf("(iv)  Median                          : %.2f\n", find_median(arr, n));

    /* (v) */
    printf("(v)   Standard deviation              : %.2f\n", find_std_dev(arr, n, mean));

    /* (vi) */
    printf("(vi)  Mode                            : %d\n", find_mode(arr, n));

    /* (vii) - work on a copy so the original array survives for later steps */
    {
        int *dup_copy = malloc((size_t)n * sizeof(int));
        for (int i = 0; i < n; ++i) dup_copy[i] = arr[i];
        int new_n = remove_duplicates(dup_copy, n);
        printf("(vii) After removing duplicates (%d unique): ", new_n);
        print_array(dup_copy, new_n);
        free(dup_copy);
    }

    /* (viii) */
    {
        int *rev_copy = malloc((size_t)n * sizeof(int));
        for (int i = 0; i < n; ++i) rev_copy[i] = arr[i];
        reverse_array(rev_copy, n);
        printf("(viii) Reversed array                 : ");
        print_array(rev_copy, n);
        free(rev_copy);
    }

    /* (ix) */
    {
        int *part_copy = malloc((size_t)n * sizeof(int));
        for (int i = 0; i < n; ++i) part_copy[i] = arr[i];
        int boundary = partition_random_pivot(part_copy, n);
        printf("(ix)  Partitioned (>=pivot first, pivot index %d): ", boundary);
        print_array(part_copy, n);
        free(part_copy);
    }

    free(arr);
    return 0;
}
