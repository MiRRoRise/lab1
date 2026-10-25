/*
 * mandelbrot_parallel.c
 *
 * Параллельная программа для нахождения заданного
 * количества точек множества Мандельброта.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mandelbrot.h"
#include "timer.h"

#define CSV_FILE "points.csv"

// Общие переменные
int        thread_count;       // количество потоков
int        npoints;            // сколько точек нужно найти
Point**    thread_points;      // thread_points[r] - буфер потока r
int*       thread_found;       // сколько точек нашел поток r
int        total_found = 0;    // общий счетчик найденных точек
pthread_mutex_t count_mutex;   // защищает total_found

// Функция потока
void* Thread_work(void* rank);
void Usage(char* prog_name);

//-----------------------------------------------------------------
int main(int argc, char* argv[]) {
   int        i, r, idx;
   int        merged_count = 0;
   Point*     merged;
   pthread_t* thread_handles;
   double     start, finish;

   if (argc != 3) Usage(argv[0]);
   thread_count = (int) strtol(argv[1], NULL, 10);
   npoints      = (int) strtol(argv[2], NULL, 10);
   if (thread_count < 1 || npoints < 1) Usage(argv[0]);

   thread_points = malloc(thread_count * sizeof(Point*));
   thread_found  = calloc(thread_count, sizeof(int));
   for (r = 0; r < thread_count; r++) {
      thread_points[r] = malloc(npoints * sizeof(Point));
      if (thread_points[r] == NULL) {
         fprintf(stderr, "Cannot allocate memory\n");
         exit(1);
      }
   }

   pthread_mutex_init(&count_mutex, NULL);
   thread_handles = malloc(thread_count * sizeof(pthread_t));

   GET_TIME(start);
   for (i = 0; i < thread_count; i++)
      pthread_create(&thread_handles[i], NULL, Thread_work, (void*) i);

   for (i = 0; i < thread_count; i++)
      pthread_join(thread_handles[i], NULL);
   GET_TIME(finish);
   printf("Elapsed time = %e seconds\n", finish - start);
   printf("Total points found = %d (requested %d)\n", total_found, npoints);

   merged = malloc(npoints * sizeof(Point));
   for (r = 0; r < thread_count && merged_count < npoints; r++) {
      for (idx = 0; idx < thread_found[r] && merged_count < npoints; idx++)
         merged[merged_count++] = thread_points[r][idx];
   }

   printf("Written %d points to %s\n",
          Write_points_csv(CSV_FILE, merged, merged_count), CSV_FILE);

   free(merged);
   for (r = 0; r < thread_count; r++)
      free(thread_points[r]);
   free(thread_points);
   free(thread_found);
   free(thread_handles);
   pthread_mutex_destroy(&count_mutex);

   return 0;
}  


void* Thread_work(void* rank) {
   int my_rank = (int) (long) rank;
   int gy, gx, my_count = 0;
   double image, real;

   for (gy = my_rank; gy < GRID && my_count < npoints; gy += thread_count) {
      pthread_mutex_lock(&count_mutex);
      if (total_found >= npoints) {
         pthread_mutex_unlock(&count_mutex);
         break;
      }
      pthread_mutex_unlock(&count_mutex);

      image = YMIN + (YMAX - YMIN) * gy / (GRID - 1);
      for (gx = 0; gx < GRID; gx++) {
         real = XMIN + (XMAX - XMIN) * gx / (GRID - 1);
         if (In_mandelbrot(real, image)) {
            int accepted = 0;
            pthread_mutex_lock(&count_mutex);

            if (total_found < npoints) {
               total_found++;
               accepted = 1;
            }

            pthread_mutex_unlock(&count_mutex);
            if (!accepted) {
               break;
            }

            thread_points[my_rank][my_count].real = real;
            thread_points[my_rank][my_count].image = image;
            my_count++;
            if (my_count >= npoints) {
               break;
            }
         }
      }
   }

   thread_found[my_rank] = my_count;
   return NULL;
} 


void Usage(char* prog_name) {
   fprintf(stderr, "usage: %s <nthreads> <npoints>\n", prog_name);
   exit(0);
}  
