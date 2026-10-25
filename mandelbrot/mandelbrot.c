/*
 * mandelbrot.c
 *
 * Функция проверки принадлежности точки множеству Мандельброта.
 * Точка c принадлежит множеству, если при итерациях z = z^2 + c 
 * величина |z| никогда не превосходит 2.
 */

#include <stdbool.h>
#include "mandelbrot.h"

bool In_mandelbrot(double real, double image) {
   double z_real = 0.0, z_image = 0.0;
   int i;

   for (i = 0; i < MAX_ITER; i++) {
      double z_real2 = z_real * z_real;
      double z_image2 = z_image * z_image;

      double next_real = z_real2 - z_image2 + real;
      double next_image = 2.0 * z_real * z_image + image;

      z_real = next_real;
      z_image = next_image;

      if (z_real * z_real + z_image * z_image > ESCAPE_RADIUS * ESCAPE_RADIUS)
         return false;
   }
   return true;
} 
