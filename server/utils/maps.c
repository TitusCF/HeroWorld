/* $Id: maps.c 11228 2009-01-24 15:41:51Z akirschbaum $ */

/* This program will generate ppm files from each of the weathermaps.
   Mostly for debugging, but curious DM's might find it useful to see
   the weather.
*/

#include <stdio.h>
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

int main(void) {
    int map[100][100];
    int x, y;
    int n, i, j, k, l, z, w, r, a;
    FILE *fp, *lp;
    int junk;
    char fn[6];

    sprintf(fn, "winddirmap");
    lp = fopen(fn, "r");
    fp = fopen("winddirmap.ppm", "w");
    fprintf(fp, "P3 %d %d 255\n", 100, 100);
    for (j = 0; j < 100; j++) {
        for (k = 0; k < 100; k++) {
            fscanf(lp, "%d ", &map[j][k]);
            junk = map[j][k];
            fprintf(lp, "%d ", map[j][k]);
            switch (junk) {
            case 0: fprintf(fp, "255 255 255 "); break;
            case 1: fprintf(fp, "0 0 0 "); break;
            case 2: fprintf(fp, "0 0 127 "); break;
            case 3: fprintf(fp, "0 0 255 "); break;
            case 4: fprintf(fp, "127 127 255 "); break;
            case 5: fprintf(fp, "127 127 127 "); break;
            case 6: fprintf(fp, "127 255 127 "); break;
            case 7: fprintf(fp, "0 255 0 "); break;
            case 8: fprintf(fp, "0 127 0 "); break;
            }
            fprintf(fp, "\n");
            fscanf(lp, "\n");
        }
    }
    fclose(fp);
    fclose(lp);

    sprintf(fn, "pressuremap");
    lp = fopen(fn, "r");
    fp = fopen("pressuremap.ppm", "w");
    fprintf(fp, "P3 %d %d 255\n", 100, 100);
    for (j = 0; j < 100; j++) {
        for (k = 0; k < 100; k++) {
            fscanf(lp, "%d ", &map[j][k]);
            junk = map[j][k];
            fprintf(lp, "%d ", map[j][k]);
            if (junk < 1000)
                fprintf(fp, "0 0 %d ", 255-(1000-junk)*3);
            else
                fprintf(fp, "0 %d 0 ", 255+(1000-junk)*3);
            fprintf(fp, "\n");
            fscanf(lp, "\n");
        }
    }
    fclose(fp);
    fclose(lp);

    sprintf(fn, "windspeedmap");
    lp = fopen(fn, "r");
    fp = fopen("windspeedmap.ppm", "w");
    fprintf(fp, "P3 %d %d 255\n", 100, 100);
    for (j = 0; j < 100; j++) {
        for (k = 0; k < 100; k++) {
            fscanf(lp, "%d ", &map[j][k]);
            junk = map[j][k];
            fprintf(lp, "%d ", map[j][k]);
            fprintf(fp, "0 %d 0 ", junk*5);
            fprintf(fp, "\n");
            fscanf(lp, "\n");
        }
    }
    fclose(fp);
    fclose(lp);

    sprintf(fn, "humidmap");
    lp = fopen(fn, "r");
    fp = fopen("humidmap.ppm", "w");
    fprintf(fp, "P3 %d %d 255\n", 100, 100);
    for (j = 0; j < 100; j++) {
        for (k = 0; k < 100; k++) {
            fscanf(lp, "%d ", &map[j][k]);
            junk = map[j][k];
            fprintf(lp, "%d ", map[j][k]);
            fprintf(fp, "0 0 %d ", 255-(junk*2));
            fprintf(fp, "\n");
            fscanf(lp, "\n");
        }
    }
    fclose(fp);
    fclose(lp);

    sprintf(fn, "temperaturemap");
    lp = fopen(fn, "r");
    fp = fopen("temperaturemap.ppm", "w");
    fprintf(fp, "P3 %d %d 255\n", 100, 100);
    for (j = 0; j < 100; j++) {
        for (k = 0; k < 100; k++) {
            fscanf(lp, "%d ", &map[j][k]);
            junk = map[j][k];
            fprintf(lp, "%d ", map[j][k]);
            if (junk < 0)
                fprintf(fp, "0 0 %d ", 255+junk*5);
            else
                fprintf(fp, "0 %d 0 ", junk*5);
            fprintf(fp, "\n");
            fscanf(lp, "\n");
        }
    }
    fclose(fp);
    fclose(lp);

    sprintf(fn, "skymap");
    lp = fopen(fn, "r");
    if (lp == NULL)
        exit(0);
    fp = fopen("skymap.ppm", "w");
    fprintf(fp, "P3 %d %d 255\n", 100, 100);
    for (j = 0; j < 100; j++) {
        for (k = 0; k < 100; k++) {
            fscanf(lp, "%d ", &map[j][k]);
            junk = map[j][k];
            fprintf(lp, "%d ", map[j][k]);
            switch (junk) {
            case 0: fprintf(fp, "0 0 255 "); break;
            case 1: fprintf(fp, "230 230 230 "); break;
            case 2: fprintf(fp, "170 170 170 "); break;
            case 3: fprintf(fp, "120 120 120 "); break;
            case 4: fprintf(fp, "80 80 80 "); break;
            case 5: fprintf(fp, "40 40 40 "); break;
            case 6: fprintf(fp, "10 10 10 "); break;
            case 7: fprintf(fp, "0 255 0 "); break;
            case 8: fprintf(fp, "255 0 0 "); break;
            case 13: fprintf(fp, "230 230 255 "); break;
            case 14: fprintf(fp, "170 170 255 "); break;
            case 15: fprintf(fp, "120 120 255 "); break;
            case 16: fprintf(fp, "80 80 255 "); break;
            }
            fprintf(fp, "\n");
            fscanf(lp, "\n");
        }
    }
    fclose(fp);
    fclose(lp);
}
