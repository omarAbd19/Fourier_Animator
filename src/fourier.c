#include "fourier.h"
#include <stdbool.h>

complex_t complex_add(complex_t *a, complex_t *b)
{
    complex_t r;
    r.real  = a->real + b->real;
    r.imag  = a->imag + b->imag;
    return r;
}

complex_t complex_multiply(complex_t *a, complex_t *b)
{
    complex_t r;
    r.real = a->real * b->real - a->imag * b->imag;
    r.imag = a->real * b->imag + a->imag * b->real;
    return r;
}

float complex_magnitude(complex_t *a)
{
    return sqrtf(a->real*a->real + a->imag*a->imag);
}

float complex_phase(complex_t *a)
{
    return atan2f(a->imag, a->real);
}

complex_t *DFT(complex_t *arr, int N)
{
    if (arr == NULL) return NULL;

    complex_t *output = (complex_t *)malloc(sizeof(complex_t) * N);

    for (int k = 0; k < N; k++) 
    {
        output[k].real = 0;
        output[k].imag = 0;

        for (int n = 0; n < N; n++)
        {
            float angle = (-2 * PI * k * n) / N;
            
            complex_t rotation_factor; 
            rotation_factor.real =  cosf(angle);
            rotation_factor.imag =  sinf(angle);

            /* (a + bi)(cos + i·sin) = (a·cos - b·sin) + i(a·sin + b·cos) */
            output[k].real += arr[n].real * rotation_factor.real - arr[n].imag * rotation_factor.imag;
            output[k].imag += arr[n].real * rotation_factor.imag + arr[n].imag * rotation_factor.real;
        }
        /* Normalize by dividing by N to get proper amplitudes */
        output[k].real /= N;
        output[k].imag /= N;
    }
    return output;
}

epicycle_t *dft_to_epicycles(complex_t *dft, int N)
{
    if (dft == NULL) return NULL;

    epicycle_t *epic = (epicycle_t *)malloc(sizeof(epicycle_t) * N);

    for (int k = 0; k < N; k++) 
    {
        epic[k].frequency = (float)k;
        epic[k].amplitude = complex_magnitude(&dft[k]);
        epic[k].phase     = complex_phase(&dft[k]);
    }
    return epic;
}

Vector2 epicycles_position(epicycle_t *epic, int N, float t)
{
    Vector2 sum = { 0, 0 };

    for (int k = 0; k < N; k++)
    {
        float phase_arg = epic[k].frequency * t + epic[k].phase;
        
        sum.x += epic[k].amplitude * cosf(phase_arg); 
        sum.y += epic[k].amplitude * sinf(phase_arg); 
    }
    return sum;
}

Vector2 draw_epicycles(epicycle_t *epic, int N, float t, float line_thickness)
{
    float x = 0;
    float y = 0;

    for (int k = 0; k < N; k++)
    {
        float prev_x = x;
        float prev_y = y;
        
        float phase_arg = epic[k].frequency * t + epic[k].phase;
        x += epic[k].amplitude * cosf(phase_arg);
        y += epic[k].amplitude * sinf(phase_arg);
        
        /* Only draw circles if amplitude is significant enough to see */
        if (epic[k].amplitude > 1.0f) {
            /* Color gradient based on index */
            unsigned char r = (unsigned char)(50 + ((k * 205) % 206));
            unsigned char g = (unsigned char)(100 + ((k * 50) % 156));
            unsigned char b = (unsigned char)(200 - ((k * 100) % 151));
            
            /* Draw the circle for this epicycle */
            DrawCircleLines((int)prev_x, (int)prev_y, epic[k].amplitude, (Color){r, g, b, 80});
            
            /* Draw line from center to point on circle (the arm) */
            DrawLineEx((Vector2){prev_x, prev_y}, (Vector2){x, y}, line_thickness, (Color){r, g, b, 180});
        }
    }
    
    /* Draw final tip position - THIS is where the line is drawn from */
    DrawCircle((int)x, (int)y, 4.0f * line_thickness, (Color){255, 100, 100, 255});
    
    return (Vector2){ x, y };
}
/* ========== Application State Functions ========== */

void app_state_init(AppState *state) {
    state->point_count = 0;
    state->trace_count = 0;
    state->dft_result = NULL;
    state->epicycles = NULL;
    
    state->t = 0.0f;
    state->speed = 1.0f;
    state->frame_time = 0.0f;
    state->line_thickness = 2.0f;
    state->current_k = 0;
    state->color_index = 0;
    state->animation_done = false;
    
    state->is_drawing = false;
    state->was_drawing = false;
    state->proceed = false;
    state->restart_clicked = false;
    
    state->num_shape_files = 0;
    state->show_file_picker = false;
    state->file_scroll = 0;
}

bool app_start_animation(AppState *state) {
    if (state->point_count <= 0) return false;
    
    /* Free any existing data */
    if (state->dft_result) { free(state->dft_result); state->dft_result = NULL; }
    if (state->epicycles)  { free(state->epicycles);  state->epicycles = NULL; }
    
    /* Allocate and populate complex array from drawing points */
    state->dft_result = (complex_t *)malloc(sizeof(complex_t) * state->point_count);
    if (!state->dft_result) return false;
    
    for (int i = 0; i < state->point_count; i++) {
        state->dft_result[i].real = state->drawing_points[i].x;
        state->dft_result[i].imag = state->drawing_points[i].y;
    }
    
    /* Compute DFT */
    state->dft_result = DFT(state->dft_result, state->point_count);
    if (!state->dft_result) return false;
    
    /* Convert to epicycles */
    state->epicycles = dft_to_epicycles(state->dft_result, state->point_count);
    if (!state->epicycles) return false;
    
    /* Reset animation state */
    state->t = 0.0f;
    state->trace_count = 0;
    state->animation_done = false;
    state->proceed = true;
    
    return true;
}

void app_reset(AppState *state) {
    if (state->dft_result) { free(state->dft_result); state->dft_result = NULL; }
    if (state->epicycles)  { free(state->epicycles);  state->epicycles = NULL; }
    
    state->point_count = 0;
    state->trace_count = 0;
    state->t = 0.0f;
    state->proceed = false;
    state->animation_done = false;
    state->current_k = 0;
    state->was_drawing = false;
    state->is_drawing = false;
    state->frame_time = 0.0f;
    state->restart_clicked = true;
}