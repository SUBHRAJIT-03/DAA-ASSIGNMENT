#include<stdio.h>
int findpartition(int arr[], int size){
    int low=0;
    int high=size-1;
    while(low<high){
        int mid=low+(high-low)/2;
        if(arr[mid]>arr[low]){
            high=mid;
        }else{
            low=mid+1;
        }
    }
    return low;

}

int main(){
    int arr[100], size, i;
    printf("Enter the size of the array: ");
    scanf("%d", &size);
    printf("Enter the elements of the array: ");
    for(i=0; i<size; i++){
        scanf("%d", &arr[i]);
    }
    int partition_point = findpartition(arr, size);
    printf("The partition point is: %d\n", partition_point);
    return 0;
}
