#ifndef NBODY_H
#define NBODY_H

#define G_CONST 6.67430e-11    // гравитационная постоянная
#define DT 3600.0              // шаг по времени, с (1 час)

typedef struct {
    double m;               // масса
    double x, y;            // координаты
    double vx, vy;          // скорости
} Particle;

/*
 * Читает файл: первая строка - n, далее n строк по 7 чисел
 * (масса, x, y, z, vx, vy, vz). Модель двумерная: z и vz читаются
 * по формату входа, но не используются.
 */
int Read_particles(const char* fname, Particle** particles_p);

/*
 * Считает силы по строкам i = start, start+stride, ...:
 * для каждой пары (i, j), j > i, вычисляет силу по закону тяготения
 * и раскладывает ее по закону Ньютона: fx[i] += F, fx[j] -= F.
 * fx и fy — локальные массивы частичных вкладов вызывающего потока.
 */
void Compute_forces(const Particle* p, int n, int start, int stride,
                    double* fx, double* fy);

/*
 * Один шаг Эйлера для частиц i = start, start+stride, ...
 * по формулам:
 *    x += vx*dt;  y += vy*dt;        (по скоростям на предыдущем шаге)
 *    vx += Fx/m*dt;  vy += Fy/m*dt;  (по силам на предыдущем шаге)
 * fx, fy - итоговые (после редукции) суммарные силы.
 */
void Euler_step(Particle* p, int n, int start, int stride,
                const double* fx, const double* fy, double dt);

#endif
