#define _CRT_SECURE_NO_WARNINGS
#include "shapes.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

/* Generate a circle */
int generate_circle(Vector2 *points, float cx, float cy, float radius, int num_points) {
    for (int i = 0; i < num_points; i++) {
        float angle = (2.0f * PI * i) / num_points;
        points[i].x = cx + radius * cosf(angle);
        points[i].y = cy + radius * sinf(angle);
    }
    return num_points;
}

/* Generate a square */
int generate_square(Vector2 *points, float cx, float cy, float size, int num_points) {
    int per_side = num_points / 4;
    int idx = 0;
    float half = size / 2;
    
    /* Top edge (left to right) */
    for (int i = 0; i < per_side && idx < num_points; i++, idx++) {
        points[idx].x = cx - half + (size * i / per_side);
        points[idx].y = cy - half;
    }
    /* Right edge (top to bottom) */
    for (int i = 0; i < per_side && idx < num_points; i++, idx++) {
        points[idx].x = cx + half;
        points[idx].y = cy - half + (size * i / per_side);
    }
    /* Bottom edge (right to left) */
    for (int i = 0; i < per_side && idx < num_points; i++, idx++) {
        points[idx].x = cx + half - (size * i / per_side);
        points[idx].y = cy + half;
    }
    /* Left edge (bottom to top) */
    for (int i = 0; i < per_side && idx < num_points; i++, idx++) {
        points[idx].x = cx - half;
        points[idx].y = cy + half - (size * i / per_side);
    }
    return idx;
}

/* Generate a star */
int generate_star(Vector2 *points, float cx, float cy, float outer_r, float inner_r, int num_spikes, int num_points) {
    int idx = 0;
    int points_per_segment = num_points / (num_spikes * 2);
    
    for (int spike = 0; spike < num_spikes * 2 && idx < num_points; spike++) {
        float angle1 = (PI * spike) / num_spikes - PI / 2;
        float angle2 = (PI * (spike + 1)) / num_spikes - PI / 2;
        float r1 = (spike % 2 == 0) ? outer_r : inner_r;
        float r2 = (spike % 2 == 0) ? inner_r : outer_r;
        
        for (int i = 0; i < points_per_segment && idx < num_points; i++, idx++) {
            float t = (float)i / points_per_segment;
            float x1 = cx + r1 * cosf(angle1);
            float y1 = cy + r1 * sinf(angle1);
            float x2 = cx + r2 * cosf(angle2);
            float y2 = cy + r2 * sinf(angle2);
            points[idx].x = x1 + t * (x2 - x1);
            points[idx].y = y1 + t * (y2 - y1);
        }
    }
    return idx;
}

/* Generate a heart shape */
int generate_heart(Vector2 *points, float cx, float cy, float size, int num_points) {
    for (int i = 0; i < num_points; i++) {
        float t = (2.0f * PI * i) / num_points;
        /* Parametric heart equations */
        float x = 16.0f * sinf(t) * sinf(t) * sinf(t);
        float y = -(13.0f * cosf(t) - 5.0f * cosf(2*t) - 2.0f * cosf(3*t) - cosf(4*t));
        points[i].x = cx + x * (size / 17.0f);
        points[i].y = cy + y * (size / 17.0f);
    }
    return num_points;
}

/* Generate infinity symbol (lemniscate) */
int generate_infinity(Vector2 *points, float cx, float cy, float size, int num_points) {
    for (int i = 0; i < num_points; i++) {
        float t = (2.0f * PI * i) / num_points;
        /* Lemniscate of Bernoulli */
        float denom = 1.0f + sinf(t) * sinf(t);
        points[i].x = cx + (size * cosf(t)) / denom;
        points[i].y = cy + (size * sinf(t) * cosf(t)) / denom;
    }
    return num_points;
}

/* Generate a spiral */
int generate_spiral(Vector2 *points, float cx, float cy, float max_radius, int num_points) {
    float turns = 3.0f;
    for (int i = 0; i < num_points; i++) {
        float t = (float)i / num_points;
        float angle = turns * 2.0f * PI * t;
        float radius = max_radius * t;
        points[i].x = cx + radius * cosf(angle);
        points[i].y = cy + radius * sinf(angle);
    }
    return num_points;
}

/* Load shape from a text file
 * File format: one "x y" coordinate per line
 * Example:
 *   100 50
 *   105 55
 *   110 60
 * Points are centered and scaled to fit around (cx, cy)
 */
int load_shape_from_file(Vector2 *points, const char *filename, float cx, float cy, float scale, int max_points) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Could not open shape file: %s\n", filename);
        return 0;
    }
    
    /* First pass: read points and find bounding box */
    float temp_x[5000], temp_y[5000];
    int count = 0;
    float min_x = 1e9f, max_x = -1e9f;
    float min_y = 1e9f, max_y = -1e9f;
    
    char line[256];
    while (fgets(line, sizeof(line), file) && count < max_points && count < 5000) {
        float x, y;
        if (sscanf(line, "%f %f", &x, &y) == 2) {
            temp_x[count] = x;
            temp_y[count] = y;
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;
            count++;
        }
    }
    fclose(file);
    
    if (count == 0) return 0;
    
    /* Calculate center and size of original shape */
    float orig_cx = (min_x + max_x) / 2.0f;
    float orig_cy = (min_y + max_y) / 2.0f;
    float width = max_x - min_x;
    float height = max_y - min_y;
    float max_dim = (width > height) ? width : height;
    
    /* Scale factor to fit shape to desired size */
    float s = (max_dim > 0) ? (scale / max_dim) : 1.0f;
    
    /* Second pass: center and scale points */
    for (int i = 0; i < count; i++) {
        points[i].x = cx + (temp_x[i] - orig_cx) * s;
        points[i].y = cy + (temp_y[i] - orig_cy) * s;
    }
    
    printf("Loaded %d points from %s\n", count, filename);
    return count;
}
