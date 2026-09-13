#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

typedef struct { int x; int y; } Point;

static int cmp_point(const void *a, const void *b) {
    const Point *p = a, *q = b;
    if (p->x != q->x) return (p->x < q->x) ? -1 : 1;
    if (p->y != q->y) return (p->y < q->y) ? -1 : 1;
    return 0;
}

static int point_exists(const Point *arr, int n, Point key) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        const Point *p = &arr[mid];
        if (p->x == key.x && p->y == key.y) return 1;
        if (p->x < key.x || (p->x == key.x && p->y < key.y)) lo = mid + 1;
        else hi = mid - 1;
    }
    return 0;
}

static int read_input_mode(void) {
    int mode = 0;
    printf("Invert the coin-triangle — minimum single-coin slides\n");
    printf("Choose input mode:\n 1) Manual (enter side length n)\n 2) Random n (1..max)");
    printf("\nEnter 1 or 2: ");
    if (scanf("%d", &mode) != 1) return -1;
    return mode;
}

static int read_manual_n(void) {
    int n = 0;
    printf("Enter side length n (positive integer): ");
    if (scanf("%d", &n) != 1) return -1;
    return n;
}

static int generate_random_n(void) {
    int maxn = 10;
    printf("Enter maximum n for random generation (default 10): ");
    if (scanf("%d", &maxn) != 1) maxn = 10;
    if (maxn < 1) maxn = 10;
    srand((unsigned)time(NULL));
    int n = (rand() % maxn) + 1;
    printf("Randomly chosen n = %d\n", n);
    return n;
}

static int solve_coin_triangle(int n) {
    if (n <= 0) {
        printf("n must be positive.\n");
        return 1;
    }

    int T = n * (n + 1) / 2;
    Point *pts = malloc(sizeof(Point) * T);
    if (!pts) {
        perror("malloc");
        return 1;
    }

    int idx = 0;
    long long sumx = 0, sumy = 0;
    for (int r = 0; r < n; ++r) {
        for (int c = 0; c <= r; ++c) {
            pts[idx].x = 2 * c - r;
            pts[idx].y = r;
            sumx += pts[idx].x;
            sumy += pts[idx].y;
            ++idx;
        }
    }

    double Cx = (double)sumx / (double)T;
    double Cy = (double)sumy / (double)T;

    qsort(pts, T, sizeof(Point), cmp_point);

    int overlap = 0;
    for (int i = 0; i < T; ++i) {
        double rx = 2.0 * Cx - (double)pts[i].x;
        double ry = 2.0 * Cy - (double)pts[i].y;
        int ix = (int)llround(rx);
        int iy = (int)llround(ry);
        Point q = { ix, iy };
        if (point_exists(pts, T, q)) overlap++;
    }

    int min_moves = T - overlap;
    printf("n = %d (rows)\n", n);
    printf("Total coins T = %d\n", T);
    printf("Coins that remain in place after 180deg rotation (overlap) = %d\n", overlap);
    printf("Minimum moves (T - overlap) = %d\n", min_moves);

    free(pts);
    return 0;
}

int main(void) {
    int mode = read_input_mode();
    if (mode == -1) return 1;

    int n = (mode == 1) ? read_manual_n() : generate_random_n();
    if (n < 0) return 1;

    return solve_coin_triangle(n);
}
