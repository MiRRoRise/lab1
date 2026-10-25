#ifndef _MANDELBROT_H_
#define _MANDELBROT_H_

#include <stdbool.h>

#define MAX_ITER 1000          // максимум итераций для проверки точки
#define ESCAPE_RADIUS 2.0      // порог модуля |z| 

// Область комплексной плоскости, покрываемая сеткой
#define GRID 2000              // сетка GRID x GRID
#define XMIN -2.0
#define XMAX  0.5
#define YMIN -1.25
#define YMAX  1.25

typedef struct {
    double real;  
    double image;  
} Point;

/* Возвращает true, если точка c = real + i*image принадлежит множеству Мандельброта */
bool In_mandelbrot(double real, double image);

/*
 * Записывает в файл fname первые count точек из массива points.
 * Возвращает количество записанных точек.
*/
int Write_points_csv(const char* fname, const Point* points, int count);

#endif 
