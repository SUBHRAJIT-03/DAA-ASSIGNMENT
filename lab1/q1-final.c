#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Structure to hold function information
typedef struct {
    const char *name;
    double (*func)(double);
    double evaluated_value;
} FunctionItem;

// Define the given functions from the table
double f_invn(double n)        { return 1.0 / n; }
double f_logn(double n)        { return log2(n); }
double f_12sqrtn(double n)     { return 12 * sqrt(n); }
double f_50sqrtn(double n)     { return 50 * pow(n, 0.5); }
double f_n051(double n)        { return pow(n, 0.51); }
double f_nlogn(double n)       { return n * log2(n); }
double f_232n(double n)        { return 4294967296.0 * n; }
double f_n2_324(double n)      { return pow(n, 2) - 324; }
double f_100n2_6n(double n)    { return 100 * pow(n, 2) + 6 * n; }
double f_2n3(double n)         { return 2 * pow(n, 3); }
double f_npowlogn(double n)    { return pow(n, log2(n)); }
double f_3n(double n)          { return pow(3, n); }

// Comparison function for qsort
int compare(const void *a, const void *b) {
    FunctionItem *itemA = (FunctionItem *)a;
    FunctionItem *itemB = (FunctionItem *)b;
    if (itemA->evaluated_value < itemB->evaluated_value) return -1;
    if (itemA->evaluated_value > itemB->evaluated_value) return 1;
    return 0;
}

int main() {
    double n_test = 100000.0;

    FunctionItem items[] = {
        {"1 / n", f_invn, 0},
        {"log2(n)", f_logn, 0},
        {"12 * sqrt(n)", f_12sqrtn, 0},
        {"50 * n^0.5", f_50sqrtn, 0},
        {"n^0.51", f_n051, 0},
        {"n * log2(n)", f_nlogn, 0},
        {"2^32 * n", f_232n, 0},
        {"n^2 - 324", f_n2_324, 0},
        {"100n^2 + 6n", f_100n2_6n, 0},
        {"2n^3", f_2n3, 0},
        {"n^(log2 n)", f_npowlogn, 0},
        {"3^n", f_3n, 0}
    };

    int num_items = sizeof(items) / sizeof(items[0]);

    // Evaluate each function at n_test
    for (int i = 0; i < num_items; i++) {
        items[i].evaluated_value = items[i].func(n_test);
    }

    // Sort by growth rate magnitude
    qsort(items, num_items, sizeof(FunctionItem), compare);

    // Print the sorted order to the console
    printf("Functions ordered by increasing growth rate at n = %.0f:\n", n_test);
    printf("----------------------------------------------------\n");
    for (int i = 0; i < num_items; i++) {
        printf("%2d. %-20s (Val: %e)\n", i + 1, items[i].name, items[i].evaluated_value);
    }

    // Open Gnuplot pipe and plot the functions
    FILE *gp = popen("gnuplot -persistent", "w");
    if (gp == NULL) {
        perror("Error opening gnuplot");
        return 1;
    }

    // Configure Gnuplot for plotting
    fprintf(gp, "set title 'Function Growth Rate Comparison'\n");
    fprintf(gp, "set xlabel 'Input Size (n)'\n");
    fprintf(gp, "set ylabel 'Operations / Time'\n");
    fprintf(gp, "set grid\n");
    fprintf(gp, "set key outside rmargin top vertical\n");
    fprintf(gp, "set logscale y\n");
    fprintf(gp, "set xrange [1:100]\n");
    fprintf(gp, "set yrange [1e-2:1e20]\n");
    
    // Plot with explicitly unique colors (lc) and line styles/dashes (dt)
    fprintf(gp, "plot "
                "x*log(x)         title 'O(n log n)'    with lines lw 2 lc rgb 'red', "
                "12*x**(1.0/2.0)  title 'O(n^{1/2})'    with lines lw 2 lc rgb 'dark-green', "
                "1.0/x            title 'O(1/n)'        with lines lw 2 lc rgb 'blue', "
                "x**(log(x))      title 'O(n^{log n})'  with lines lw 2 lc rgb 'purple', "
                "100*x**2 + 6*x   title 'O(n^2)'        with lines lw 2 lc rgb 'dark-orange', "
                "x**(0.51)        title 'O(n^{0.51})'   with lines lw 2 lc rgb 'cyan', "
                "x**2 - 324       title 'O(n^2 - 324)'  with lines lw 2 lc rgb 'magenta', "
                "50*x**(0.50)     title 'O(n^{1/2}) alt' with lines lw 2 lc rgb 'brown' dt 2, "
                "2*x**3           title 'O(n^3)'        with lines lw 2 lc rgb 'coral', "
                "3**x             title 'O(3^n)'        with lines lw 2 lc rgb 'sea-green' dt 3, "
                "x*2**32          title 'O(n*2^{32})'   with lines lw 2 lc rgb 'gold', "
                "log(x)           title 'O(log n)'      with lines lw 2 lc rgb 'dark-turquoise'\n");

    fflush(gp);
    pclose(gp);

    return 0;
}
