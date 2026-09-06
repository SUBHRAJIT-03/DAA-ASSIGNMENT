#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* =====================================================================
   Helpers: allocation, filling, printing
   ===================================================================== */

static double **alloc_matrix(int n) {
    double **m = malloc((size_t)n * sizeof(double *));
    for (int i = 0; i < n; ++i) {
        m[i] = malloc((size_t)n * sizeof(double));
    }
    return m;
}

static void free_matrix(double **m, int n) {
    for (int i = 0; i < n; ++i) free(m[i]);
    free(m);
}

static void fill_random(double **m, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            m[i][j] = (double)(rand() % 19 - 9); /* integers in [-9, 9] */
        }
    }
}

static void fill_user_input(double **m, int n, const char *label) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            printf("Enter %s[%d][%d]: ", label, i, j);
            scanf("%lf", &m[i][j]);
        }
    }
}

static void print_matrix(double **m, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            printf("%8.2f ", m[i][j]);
        }
        printf("\n");
    }
}

static double **copy_matrix(double **m, int n) {
    double **c = alloc_matrix(n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            c[i][j] = m[i][j];
    return c;
}

/* =====================================================================
   (i) Matrix addition -- O(n^2)
   Every one of the n^2 output cells is a single independent addition.
   ===================================================================== */
static double **add_matrices(double **A, double **B, int n) {
    double **C = alloc_matrix(n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            C[i][j] = A[i][j] + B[i][j];
    return C;
}

/* =====================================================================
   (ii) Matrix multiplication -- O(n^3) (naive triple loop)
   Each of the n^2 output cells needs a length-n dot product.
   ===================================================================== */
static double **multiply_matrices(double **A, double **B, int n) {
    double **C = alloc_matrix(n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
    return C;
}

/* =====================================================================
   (iii) Zero-matrix check -- O(n^2) worst case
   Scans every cell; can exit early the moment a non-zero entry is
   found, so the best case is O(1) but the worst case (a genuine zero
   matrix, or a non-zero value only in the very last cell) is O(n^2).
   ===================================================================== */
static int is_zero_matrix(double **m, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (m[i][j] != 0.0) return 0;
        }
    }
    return 1;
}

/* =====================================================================
   (iv) Symmetric-matrix check -- O(n^2)
   Only the n(n-1)/2 pairs above the diagonal need comparing against
   their mirror image below it -- still Theta(n^2), just with half
   the constant of a naive full scan.
   ===================================================================== */
static int is_symmetric(double **m, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (m[i][j] != m[j][i]) return 0;
        }
    }
    return 1;
}

/* =====================================================================
   (v) Determinant -- Gaussian elimination with partial pivoting, O(n^3)
   Reduces a *copy* of the matrix to upper-triangular form; the
   determinant is then the product of the pivots (with a sign flip per
   row swap). This replaces the O(n!) cost of cofactor expansion.
   ===================================================================== */
static double compute_determinant(double **A, int n) {
    double **m = copy_matrix(A, n);
    double det = 1.0;

    for (int col = 0; col < n; ++col) {
        int pivot_row = col;
        double max_val = fabs(m[col][col]);
        for (int row = col + 1; row < n; ++row) {
            if (fabs(m[row][col]) > max_val) {
                max_val = fabs(m[row][col]);
                pivot_row = row;
            }
        }

        if (max_val < 1e-12) { /* singular matrix, determinant is 0 */
            det = 0.0;
            break;
        }

        if (pivot_row != col) {
            double *tmp = m[pivot_row];
            m[pivot_row] = m[col];
            m[col] = tmp;
            det = -det;
        }

        det *= m[col][col];

        for (int row = col + 1; row < n; ++row) {
            double factor = m[row][col] / m[col][col];
            for (int k = col; k < n; ++k) {
                m[row][k] -= factor * m[col][k];
            }
        }
    }

    free_matrix(m, n);
    return det;
}

/* =====================================================================
   (vi) Transpose in place -- O(n^2) time, O(1) extra space
   Only possible without a second matrix because the matrix is square:
   swap every (i, j) above the diagonal with its mirror (j, i).
   ===================================================================== */
static void transpose_in_place(double **m, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double tmp = m[i][j];
            m[i][j] = m[j][i];
            m[j][i] = tmp;
        }
    }
}

/* =====================================================================
   (vii) Dominant eigenvalue / eigenvector -- Power Iteration
   There is no general closed-form algorithm for the roots of an
   n x n characteristic polynomial once n >= 5 (Abel-Ruffini theorem),
   so exact eigenvalues cannot be "computed" the way a determinant can
   -- practical algorithms are iterative. Power Iteration repeatedly
   multiplies a vector by the matrix and renormalises it; it converges
   to the eigenvector of the largest-magnitude eigenvalue whenever one
   exists (real-valued and strictly larger in magnitude than every
   other eigenvalue) and the starting vector isn't perpendicular to it.
   Each iteration costs O(n^2) (one matrix-vector product), so the
   total cost is O(k * n^2) for k iterations -- k is not a function of
   n alone, it depends on how well-separated the eigenvalues are, which
   is why this is fundamentally a numerical/iterative process rather
   than a fixed-degree polynomial-time algorithm.

   A general real (non-symmetric) matrix can have complex-conjugate
   eigenvalue pairs, in which case power iteration on it will not
   converge to a single real vector at all -- it oscillates instead.
   Real *symmetric* matrices are guaranteed to have only real
   eigenvalues, so to give power iteration a fair, always-checkable
   input, the driver below runs it on S = (A + A^T) / 2 rather than on
   A directly. Building S is itself just O(n^2).
   ===================================================================== */
static double **symmetrize(double **A, int n) {
    double **S = alloc_matrix(n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            S[i][j] = 0.5 * (A[i][j] + A[j][i]);
    return S;
}

static double power_iteration(double **A, int n, double *eigenvector,
                               int max_iter, double tol) {
    double *v = malloc((size_t)n * sizeof(double));
    double *w = malloc((size_t)n * sizeof(double));

    for (int i = 0; i < n; ++i) v[i] = 1.0; /* arbitrary starting vector */

    double norm = 0.0;
    for (int i = 0; i < n; ++i) norm += v[i] * v[i];
    norm = sqrt(norm);
    for (int i = 0; i < n; ++i) v[i] /= norm;

    double eigenvalue = 0.0;

    for (int iter = 0; iter < max_iter; ++iter) {
        for (int i = 0; i < n; ++i) {
            double sum = 0.0;
            for (int j = 0; j < n; ++j) sum += A[i][j] * v[j];
            w[i] = sum;
        }

        /* Rayleigh quotient with the *unit* vector v gives the eigenvalue estimate */
        double new_eigenvalue = 0.0;
        for (int i = 0; i < n; ++i) new_eigenvalue += v[i] * w[i];

        norm = 0.0;
        for (int i = 0; i < n; ++i) norm += w[i] * w[i];
        norm = sqrt(norm);

        if (norm < 1e-12) break; /* w collapsed to (near) zero */

        for (int i = 0; i < n; ++i) v[i] = w[i] / norm;

        if (fabs(new_eigenvalue - eigenvalue) < tol) {
            eigenvalue = new_eigenvalue;
            break;
        }
        eigenvalue = new_eigenvalue;
    }

    for (int i = 0; i < n; ++i) eigenvector[i] = v[i];

    free(v);
    free(w);
    return eigenvalue;
}

/* =====================================================================
   main
   ===================================================================== */
int main(void) {
    int n, choice;
    srand((unsigned int)time(NULL));

    printf("Enter the size of the square matrices (n): ");
    scanf("%d", &n);

    if (n <= 0) {
        printf("Invalid size.\n");
        return 0;
    }

    double **A = alloc_matrix(n);
    double **B = alloc_matrix(n);

    printf("Choose data input method:\n");
    printf("1. Random data\n");
    printf("2. User input\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    if (choice == 1) {
        fill_random(A, n);
        fill_random(B, n);
    } else {
        fill_user_input(A, n, "A");
        fill_user_input(B, n, "B");
    }

    printf("\nMatrix A:\n");
    print_matrix(A, n);
    printf("\nMatrix B:\n");
    print_matrix(B, n);

    /* (i) */
    double **sum = add_matrices(A, B, n);
    printf("\n(i)   A + B:\n");
    print_matrix(sum, n);
    free_matrix(sum, n);

    /* (ii) */
    double **product = multiply_matrices(A, B, n);
    printf("\n(ii)  A * B:\n");
    print_matrix(product, n);
    free_matrix(product, n);

    /* (iii) */
    printf("\n(iii) Is A a zero matrix?      : %s\n", is_zero_matrix(A, n) ? "Yes" : "No");

    /* (iv) */
    printf("(iv)  Is A a symmetric matrix? : %s\n", is_symmetric(A, n) ? "Yes" : "No");

    /* (v) */
    printf("(v)   Determinant of A         : %.4f\n", compute_determinant(A, n));

    /* (vi) - work on a copy so A survives for the last step */
    {
        double **A_t = copy_matrix(A, n);
        transpose_in_place(A_t, n);
        printf("(vi)  Transpose of A (in place on a copy):\n");
        print_matrix(A_t, n);
        free_matrix(A_t, n);
    }

    /* (vii) - power iteration needs a real dominant eigenvalue to be
       guaranteed to converge, so it runs on the symmetric matrix
       S = (A + A^T) / 2 built from A rather than on A itself */
    {
        double **S = symmetrize(A, n);
        printf("(vii) S = (A + A^T) / 2, used because it is guaranteed to have real eigenvalues:\n");
        print_matrix(S, n);

        double *eigenvector = malloc((size_t)n * sizeof(double));
        double eigenvalue = power_iteration(S, n, eigenvector, 1000, 1e-9);
        printf("      Dominant eigenvalue of S : %.4f\n", eigenvalue);
        printf("      Corresponding eigenvector: [ ");
        for (int i = 0; i < n; ++i) printf("%.4f ", eigenvector[i]);
        printf("]\n");

        free(eigenvector);
        free_matrix(S, n);
    }

    free_matrix(A, n);
    free_matrix(B, n);
    return 0;
}
