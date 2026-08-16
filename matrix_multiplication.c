#include <stdio.h>
#include <stdlib.h>

// Function to allocate a matrix
int **allocateMatrix(int n) {
    int **mat = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
        mat[i] = (int *)malloc(n * sizeof(int));
    return mat;
}

// Function to free a matrix
void freeMatrix(int **mat, int n) {
    for (int i = 0; i < n; i++)
        free(mat[i]);
    free(mat);
}

// Matrix addition
void add(int **A, int **B, int **C, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            C[i][j] = A[i][j] + B[i][j];
}

// Matrix subtraction
void subtract(int **A, int **B, int **C, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            C[i][j] = A[i][j] - B[i][j];
}

// Strassen multiplication
void strassen(int **A, int **B, int **C, int n) {
    if (n == 1) {
        C[0][0] = A[0][0] * B[0][0];
        return;
    }

    int k = n / 2;

    // Allocate submatrices
    int **A11 = allocateMatrix(k);
    int **A12 = allocateMatrix(k);
    int **A21 = allocateMatrix(k);
    int **A22 = allocateMatrix(k);

    int **B11 = allocateMatrix(k);
    int **B12 = allocateMatrix(k);
    int **B21 = allocateMatrix(k);
    int **B22 = allocateMatrix(k);

    int **M1 = allocateMatrix(k);
    int **M2 = allocateMatrix(k);
    int **M3 = allocateMatrix(k);
    int **M4 = allocateMatrix(k);
    int **M5 = allocateMatrix(k);
    int **M6 = allocateMatrix(k);
    int **M7 = allocateMatrix(k);

    int **T1 = allocateMatrix(k);
    int **T2 = allocateMatrix(k);

    // Divide matrices
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            A11[i][j] = A[i][j];
            A12[i][j] = A[i][j + k];
            A21[i][j] = A[i + k][j];
            A22[i][j] = A[i + k][j + k];

            B11[i][j] = B[i][j];
            B12[i][j] = B[i][j + k];
            B21[i][j] = B[i + k][j];
            B22[i][j] = B[i + k][j + k];
        }
    }

    // M1 = (A11 + A22)(B11 + B22)
    add(A11, A22, T1, k);
    add(B11, B22, T2, k);
    strassen(T1, T2, M1, k);

    // M2 = (A21 + A22)B11
    add(A21, A22, T1, k);
    strassen(T1, B11, M2, k);

    // M3 = A11(B12 - B22)
    subtract(B12, B22, T2, k);
    strassen(A11, T2, M3, k);

    // M4 = A22(B21 - B11)
    subtract(B21, B11, T2, k);
    strassen(A22, T2, M4, k);

    // M5 = (A11 + A12)B22
    add(A11, A12, T1, k);
    strassen(T1, B22, M5, k);

    // M6 = (A21 - A11)(B11 + B12)
    subtract(A21, A11, T1, k);
    add(B11, B12, T2, k);
    strassen(T1, T2, M6, k);

    // M7 = (A12 - A22)(B21 + B22)
    subtract(A12, A22, T1, k);
    add(B21, B22, T2, k);
    strassen(T1, T2, M7, k);

    // Combine results
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            C[i][j] = M1[i][j] + M4[i][j] - M5[i][j] + M7[i][j];
            C[i][j + k] = M3[i][j] + M5[i][j];
            C[i + k][j] = M2[i][j] + M4[i][j];
            C[i + k][j + k] = M1[i][j] - M2[i][j] + M3[i][j] + M6[i][j];
        }
    }

    // Free memory
    freeMatrix(A11, k); freeMatrix(A12, k);
    freeMatrix(A21, k); freeMatrix(A22, k);
    freeMatrix(B11, k); freeMatrix(B12, k);
    freeMatrix(B21, k); freeMatrix(B22, k);

    freeMatrix(M1, k); freeMatrix(M2, k);
    freeMatrix(M3, k); freeMatrix(M4, k);
    freeMatrix(M5, k); freeMatrix(M6, k);
    freeMatrix(M7, k);

    freeMatrix(T1, k); freeMatrix(T2, k);
}

int main() {
    int n;
    printf("Enter matrix size (power of 2): ");
    scanf("%d", &n);

    int **A = allocateMatrix(n);
    int **B = allocateMatrix(n);
    int **C = allocateMatrix(n);

    printf("Enter Matrix A:\n");
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            scanf("%d", &A[i][j]);

    printf("Enter Matrix B:\n");
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            scanf("%d", &B[i][j]);

    strassen(A, B, C, n);

    printf("\nResult Matrix:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            printf("%d ", C[i][j]);
        printf("\n");
    }

    freeMatrix(A, n);
    freeMatrix(B, n);
    freeMatrix(C, n);

    return 0;
}