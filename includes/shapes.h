/**
 * Auto-draw shape generators for Fourier Animator
 * 
 * These functions generate points along various shapes that can be
 * decomposed into Fourier series and animated with epicycles.
 */

#ifndef SHAPES_H
#define SHAPES_H

#include "raylib.h"

/* Shape generation functions - return number of points generated */
int generate_circle(Vector2 *points, float cx, float cy, float radius, int num_points);
int generate_square(Vector2 *points, float cx, float cy, float size, int num_points);
int generate_star(Vector2 *points, float cx, float cy, float outer_r, float inner_r, int num_spikes, int num_points);
int generate_heart(Vector2 *points, float cx, float cy, float size, int num_points);
int generate_infinity(Vector2 *points, float cx, float cy, float size, int num_points);
int generate_spiral(Vector2 *points, float cx, float cy, float max_radius, int num_points);

/* Load shape from a text file (one "x y" coordinate per line) */
int load_shape_from_file(Vector2 *points, const char *filename, float cx, float cy, float scale, int max_points);

/* Load shape from an SVG file (extracts first path element) */
int load_svg_file(Vector2 *points, const char *filename, float cx, float cy, float scale, int max_points);

#endif
