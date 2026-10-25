/*
 * nbody.c
 *
 * Реализация модели задачи N тел: чтение входных данных, вычисление
 * сил по парам с учетом третьего закона Ньютона и один шаг Эйлера
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "nbody.h"

#define SOFTENING2 1.0e-12   // защита от деления на 0 при совпадении координат 

int Read_particles(const char* fname, Particle** particles_p) {
   int n, k;
   FILE* in = fopen(fname, "r");

   if (in == NULL) {
      fprintf(stderr, "Cannot open %s\n", fname);
      return -1;
   }
   if (fscanf(in, "%d", &n) != 1 || n < 1) {
      fprintf(stderr, "Bad particle count in %s\n", fname);
      fclose(in);
      return -1;
   }

   *particles_p = malloc(n * sizeof(Particle));
   if (*particles_p == NULL) {
      fprintf(stderr, "Cannot allocate memory\n");
      fclose(in);
      return -1;
   }

   for (k = 0; k < n; k++) {
      Particle* q = &(*particles_p)[k];
      double z, vz;   
      if (fscanf(in, "%lf %lf %lf %lf %lf %lf %lf",
                 &q->m, &q->x, &q->y, &z, &q->vx, &q->vy, &vz) != 7) {
         fprintf(stderr, "Bad line %d in %s\n", k + 2, fname);
         free(*particles_p);
         fclose(in);
         return -1;
      }
   }

   fclose(in);
   return n;
} 


void Compute_forces(const Particle* p, int n, int start, int stride,
                    double* fx, double* fy) {
   int i, j;

   for (i = start; i < n; i += stride) {
      for (j = i + 1; j < n; j++) {
         double dx = p[j].x - p[i].x;
         double dy = p[j].y - p[i].y;
         double r2 = dx * dx + dy * dy + SOFTENING2;
         double inv_r3 = 1.0 / (r2 * sqrt(r2));
         double f = G_CONST * p[i].m * p[j].m * inv_r3;

         fx[i] += f * dx;
         fy[i] += f * dy;
         fx[j] -= f * dx; 
         fy[j] -= f * dy;
      }
   }
} 


void Euler_step(Particle* p, int n, int start, int stride,
                const double* fx, const double* fy, double dt) {
   int i;

   for (i = start; i < n; i += stride) {
      p[i].x += p[i].vx * dt;
      p[i].y += p[i].vy * dt;

      p[i].vx += fx[i] / p[i].m * dt;
      p[i].vy += fy[i] / p[i].m * dt;
   }
}  
