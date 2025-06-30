#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef struct {
    double min_x;
    double max_x;
    double step;
    double *values;
    double min_y;
    double max_y;
    int size_x;
    int size_y;
} Table;

int cx(Table *t, double relative) {
    return round((-t->min_x + relative) / t->step);
}

int cy(Table *t, double relative) {
    return round((t->max_y - relative) / t->step);
}

void coord_sys(Table *t, uint8_t c[][t->size_x]) {
    int x_0 = cx(t, 0);
    int y_0 = cy(t, 0);
    if(x_0 >= 0) {
        for(int h = 0; h < t->size_y - 1; h++) {
            c[h][x_0] = 90;
        }
    }
    if(y_0 <= t->size_y) {
        for(int w = 0; w < t->size_x; w++) {
            c[y_0][w] = 90;
        }
    }
}

void plot(Table *t) {
    t->size_y = round((t->max_y - t->min_y) / t->step + 2);

    // init canvas
    uint8_t c[t->size_y][t->size_x];
    memset(c, 0, t->size_y * t->size_x * sizeof(uint8_t));

    // draw the coordinate system
    coord_sys(t, c);

    // draw the points from the table
    int a = 0;
    for(double x = t->min_x; x < t->max_x; x += t->step) {
        double y = t->values[a];
        a++;
        if(isnan(y) || isinf(y)) { continue; }
        c[cy(t, y)][cx(t, x)] = 91;
    }

    // print canvas to screen
    printf("\n");
    for(int h = 0; h < t->size_y - 1; h += 2) {
        printf("  ");
        for(int w = 0; w < t->size_x; w++) {
            if(c[h][w] != 0 && c[h+1][w] != 0) {
                printf("\x1b[%d;%dm▀\x1b[0m", c[h][w], 10 + c[h + 1][w]);
            }
            else if(c[h][w] != 0 && c[h+1][w] == 0) {
                printf("\x1b[%dm▀\x1b[0m", c[h][w]);
            }
            else if(c[h][w] == 0 && c[h+1][w] != 0) {
                printf("\x1b[%dm▄\x1b[0m", c[h+1][w]);
            }
            else {
                printf(" ");
            }
        }
        printf("\n");
    }
}
