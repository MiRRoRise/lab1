/*
 * csv.c
 *
 * Вывод траекторий частиц в CSV
 */

#include <stdio.h>
#include "nbody.h"

/* Открывает файл и пишет заголовок t,x1,y1,...,xn,yn.
 * Возвращает NULL при ошибке. */
FILE* Trajectory_open(const char* fname, int n) {
   int k;
   FILE* csv = fopen(fname, "w");

   if (csv == NULL) {
      fprintf(stderr, "Cannot open %s\n", fname);
      return NULL;
   }

   fprintf(csv, "t");
   for (k = 1; k <= n; k++)
      fprintf(csv, ",x%d,y%d", k, k);
   fprintf(csv, "\n");

   return csv;
} 


// Пишет одну строку: время t и проекции x, y всех частиц.
void Trajectory_write_row(FILE* csv, double t, const Particle* p, int n) {
   int k;

   fprintf(csv, "%.10f", t);
   for (k = 0; k < n; k++)
      fprintf(csv, ",%.10f,%.10f", p[k].x, p[k].y);
   fprintf(csv, "\n");
}  


void Trajectory_close(FILE* csv) {
   fclose(csv);
}  
