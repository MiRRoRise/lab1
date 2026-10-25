/*
 * parallel_nbody.c
 *
 * Параллельная программа для задачи N тел
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include "nbody.h"
#include "timer.h"

#define CSV_FILE "trajectories.csv"
#define DEFAULT_THREADS 4

// Вывод траекторий в CSV 
FILE*  Trajectory_open(const char* fname, int n);
void   Trajectory_write_row(FILE* csv, double t, const Particle* p, int n);
void   Trajectory_close(FILE* csv);

// Общие переменные
int         thread_count;      // количество потоков
int         n;                 // количество частиц
int         nsteps;            // количество шагов ceil(t_end / DT)
Particle*   particles;         // массив частиц (общий)
double*     fx_all;            // fx_all[rank*n+i] - вклад потока rank в силу i
double*     fy_all;            // (по оси x и y соответственно)
double*     fx_red;            // fx_red[i] - итоговая суммарная сила на i
double*     fy_red;            // (после редукции локальных вкладов)
FILE*       csv_file;          // выходной CSV 
pthread_barrier_t step_barrier;// барьер между фазами шага

// Функция потока
void* Thread_work(void* rank);
void Usage(char* prog_name);

//-----------------------------------------------------------------
int main(int argc, char* argv[]) {
   int        i;
   double     t_end;
   const char* input_file;
   pthread_t* thread_handles;
   double     start, finish;

   if (argc != 3 && argc != 4) Usage(argv[0]);
   t_end      = strtod(argv[1], NULL);
   input_file = argv[2];
   if (t_end <= 0.0) Usage(argv[0]);

   thread_count = (argc == 4) ? (int) strtol(argv[3], NULL, 10) : DEFAULT_THREADS;
   if (thread_count < 1) Usage(argv[0]);

   n = Read_particles(input_file, &particles);
   if (n < 0) {
      fprintf(stderr, "Cannot read %s\n", input_file);
      exit(1);
   }
   nsteps = (int) ceil(t_end / DT);
   if (nsteps < 1) Usage(argv[0]);

   fx_all = calloc((size_t) thread_count * n, sizeof(double));
   fy_all = calloc((size_t) thread_count * n, sizeof(double));
   fx_red = calloc((size_t) n, sizeof(double));
   fy_red = calloc((size_t) n, sizeof(double));
   if (fx_all == NULL || fy_all == NULL || fx_red == NULL || fy_red == NULL) {
      fprintf(stderr, "Cannot allocate memory\n");
      exit(1);
   }

   csv_file = Trajectory_open(CSV_FILE, n);
   if (csv_file == NULL)
      exit(1);

   pthread_barrier_init(&step_barrier, NULL, thread_count);
   thread_handles = malloc(thread_count * sizeof(pthread_t));

   GET_TIME(start);
   for (i = 0; i < thread_count; i++)
      pthread_create(&thread_handles[i], NULL, Thread_work, (void*) i);

   for (i = 0; i < thread_count; i++)
      pthread_join(thread_handles[i], NULL);
   GET_TIME(finish);
   printf("Elapsed time = %e seconds\n", finish - start);
   printf("Simulated %d steps of dt=%g up to t=%g for %d particles\n",
          nsteps, DT, nsteps * DT, n);

   Trajectory_close(csv_file);
   free(particles);
   free(fx_all);
   free(fy_all);
   free(fx_red);
   free(fy_red);
   free(thread_handles);
   pthread_barrier_destroy(&step_barrier);

   return 0;
} 


void* Thread_work(void* rank) {
   int my_rank = (int) (long) rank;
   int step, i, r;
   double* my_fx = fx_all + (size_t) my_rank * n;  
   double* my_fy = fy_all + (size_t) my_rank * n;

   // Силы F(0) по начальным координатам
   memset(my_fx, 0, (size_t) n * sizeof(double));
   memset(my_fy, 0, (size_t) n * sizeof(double));
   Compute_forces(particles, n, my_rank, thread_count, my_fx, my_fy);

   // Ждём, пока все потоки закончат вычислять свои вклады в силы.
   pthread_barrier_wait(&step_barrier);

   for (step = 0; step < nsteps; step++) {
      if (my_rank == 0)
         Trajectory_write_row(csv_file, step * DT, particles, n);
      pthread_barrier_wait(&step_barrier);  

      // Каждый поток суммирует вклады всех потоков для своих индексов i.
      // Полученные полные силы сохраняются в fx_red и fy_red.
      for (i = my_rank; i < n; i += thread_count) {
         double Fx = 0.0, Fy = 0.0;
         for (r = 0; r < thread_count; r++) {
            Fx += fx_all[(size_t) r * n + i];
            Fy += fy_all[(size_t) r * n + i];
         }
         fx_red[i] = Fx;
         fy_red[i] = Fy;
      }
      Euler_step(particles, n, my_rank, thread_count, fx_red, fy_red, DT);
      pthread_barrier_wait(&step_barrier);   // все новые координаты готовы

      // Силы для следующего шага по обновленным координатам
      memset(my_fx, 0, (size_t) n * sizeof(double));
      memset(my_fy, 0, (size_t) n * sizeof(double));
      Compute_forces(particles, n, my_rank, thread_count, my_fx, my_fy);
      pthread_barrier_wait(&step_barrier);   // F(t+dt) посчитаны всеми
   }

   if (my_rank == 0)
      Trajectory_write_row(csv_file, nsteps * DT, particles, n);

   return NULL;
}  


void Usage(char* prog_name) {
   fprintf(stderr, "usage: %s <t_end> <input_file> [nthreads]\n", prog_name);
   exit(0);
} 
