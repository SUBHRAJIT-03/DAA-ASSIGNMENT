/*Method 1: Brute Force 
Algorithm
Read n.
Read the n numbers into an array.
Compare every element with every other element.
If two elements are equal, a duplicate exists.
C Program*/
#include <stdio.h>

int main() {
    int n;
    printf("Enter the number of elements: ");
    scanf("%d", &n);

    int arr[n];

    printf("Enter the elements:\n");
    for (int i = 0; i < n; i++) {
        scanf("%d", &arr[i]);
    }

    int duplicate = 0;

    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (arr[i] == arr[j]) {
                duplicate = 1;
                break;
            }
        }
        if (duplicate)
            break;
    }

    if (duplicate)
        printf("Duplicate elements found.\n");
    else
        printf("All elements are unique.\n");

    return 0;
}
/*Time Complexity
Outer loop: n
Inner loop: n

Therefore,

Time Complexity: O(n²)
Space Complexity: O(1)
What happens for a sufficiently large value of n?

For very large n, this method becomes inefficient because it compares almost every pair of elements.

For example:

n = 1,000 → about 500,000 comparisons
n = 100,000 → about 5 billion comparisons

So the execution time increases rapidly.

Conclusion

The brute-force method has O(n²) time complexity, making it unsuitable for sufficiently large values of n. For large datasets, a more efficient approach such as sorting (O(n log n)) followed by checking adjacent elements, or using a hash table (O(n) average time), is preferred.

If this is for an algorithm/DAA exam, you can write:

Conclusion:
"The above method correctly detects duplicates but has O(n²) time complexity. As the value of n becomes very large, the number of comparisons increases quadratically, making the algorithm inefficient. Therefore, for large inputs, algorithms based on sorting (O(n log n)) or hashing (O(n)) are recommended.*/