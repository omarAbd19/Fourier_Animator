/**
 * Any closed path (like a hand-drawn shape) can be represented 
 * as a sum of rotating circles (epicycles) of different sizes, 
 * speeds, and starting angles. This is the essence of the 
 * Discrete Fourier Transform (DFT).
 * 
 * The Three Major Phases
 * - Input Capture — Collect the user's drawing as a sequence of 2D points over time
 * - Fourier Decomposition — Apply the DFT to convert those points into frequency components (epicycles)
 * - Reconstruction & Animation — Use the epicycles to trace the original path with rotating circles
 * 
 * A 2D point (x, y) can be represented as a single complex number: x + iy, where i is the imaginary unit.
 */

/*
 * DFT Explanation:
 *
 * The Discrete Fourier Transform (DFT) converts a sequence of N complex numbers
 * from the time domain into the frequency domain.
 *
 * Formula:
 *   X[k] = sum_{n=0}^{N-1} x[n] * e^(-i 2*pi*k*n/N),   for k = 0..N-1
 *
 * Here:
 *   x[n] = input[n].real + input[n].imag * i   // time-domain sample
 *   e^(-i*theta) = cos(theta) - i*sin(theta)   // complex exponential
 *
 * To implement in C:
 *   1. Compute angle = 2 * PI * k * n / N
 *   2. Compute cosA = cos(angle) and sinA = sin(angle)
 *   3. Multiply input[n] by e^(-i*angle) using complex multiplication:
 *
 *      Let x[n] = a + b*i, e^(-i*angle) = cosA - i*sinA
 *
 *      Real part:   a*cosA + b*sinA
 *      Imag part:  -a*sinA + b*cosA
 *
 *   4. Sum these contributions for all n to get X[k]
 *
 * Notes:
 *   - The negative sign in the exponent gives the standard DFT.
 *   - The result X[k] is a complex number representing amplitude and phase.
 *   - Magnitude = sqrt(real^2 + imag^2)
 *   - Phase (angle) = atan2(imag, real)
 */


#ifndef FOURIER_H
#define FOURIER_H

#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define WINDOW_WIDTH  1400
#define WINDOW_HEIGHT 1000
#define WINDOW_TITLE  "Fourier Animator"

#define DRAWING_POINTS_MAX  5000

/* Shape loading constants */
#define DEFAULT_SHAPE_SIZE    250.0f
#define DEFAULT_LOAD_SCALE    500.0f
#define DEFAULT_SHAPE_POINTS  500

/* File browser constants */
#define MAX_SHAPE_FILES    20
#define MAX_FILENAME_LEN   64

/* ========== Core Types ========== */

typedef struct {
    float real;
    float imag;
} complex_t;

typedef struct {
    float frequency;
    float amplitude;
    float phase;
} epicycle_t;

/* ========== Application State ========== */

typedef struct {
    /* Drawing state */
    Vector2 drawing_points[DRAWING_POINTS_MAX];
    Vector2 trace_points[DRAWING_POINTS_MAX];
    int point_count;
    int trace_count;
    
    /* Fourier data */
    complex_t  *dft_result;
    epicycle_t *epicycles;
    
    /* Animation state */
    float t;
    float speed;
    float frame_time;
    float line_thickness;
    int current_k;
    int color_index;      /* Index into color presets */
    bool animation_done;
    
    /* Input state */
    bool is_drawing;
    bool was_drawing;
    bool proceed;
    bool restart_clicked;
    
    /* File browser state */
    char shape_files[MAX_SHAPE_FILES][MAX_FILENAME_LEN];
    int num_shape_files;
    bool show_file_picker;
    int file_scroll;
} AppState;

/* ========== Complex Math Functions ========== */

complex_t complex_add(complex_t *a, complex_t *b);
complex_t complex_multiply(complex_t *a, complex_t *b);
float     complex_magnitude(complex_t *a);
float     complex_phase(complex_t *a);

complex_t  *DFT(complex_t *arr, int N);
epicycle_t *dft_to_epicycles(complex_t *dft, int N);
Vector2     epicycles_position(epicycle_t *epic, int N, float t);
Vector2     draw_epicycles(epicycle_t *epic, int N, float t, float line_thickness);

/* ========== Application State Functions ========== */

/**
 * Initialize the application state with default values.
 */
void app_state_init(AppState *state);

/**
 * Start the Fourier animation from the current drawing points.
 * Computes DFT and sets up epicycles.
 * 
 * @param state   Application state
 * @return        true if animation started successfully
 */
bool app_start_animation(AppState *state);

/**
 * Reset the application state to allow new drawing.
 */
void app_reset(AppState *state);

#endif