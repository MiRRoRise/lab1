/*
 * csv.c
 *
 * Функция вывода точек в файл CSV.
 */

#include <stdio.h>
#include "mandelbrot.h"

int Write_points_csv(const char* fname, const Point* points, int count) {
   int i;
   FILE* csv = fopen(fname, "w");

   if (csv == NULL) {
      fprintf(stderr, "Cannot open %s\n", fname);
      return 0;
   }

   fprintf(csv, "re,im\n");
   for (i = 0; i < count; i++)
      fprintf(csv, "%.10f,%.10f\n", points[i].real, points[i].image);

   fclose(csv);
   return count;
}  
