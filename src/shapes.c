#define _CRT_SECURE_NO_WARNINGS
#include "shapes.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

/* ========== SVG PATH PARSER ========== */

/* Helper: skip whitespace and commas */
static const char *skip_ws(const char *p) {
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ',')) p++;
    return p;
}

/* Helper: parse a float number from string */
static const char *parse_float(const char *p, float *out) {
    p = skip_ws(p);
    char *end;
    *out = strtof(p, &end);
    return end;
}

/* Helper: add point to array with bounds checking */
static int add_point(float *tx, float *ty, int *count, float x, float y, int max) {
    if (*count < max) {
        tx[*count] = x;
        ty[*count] = y;
        (*count)++;
        return 1;
    }
    return 0;
}

/* Helper: cubic bezier interpolation */
static void cubic_bezier(float *tx, float *ty, int *count, int max,
                         float x0, float y0, float x1, float y1, 
                         float x2, float y2, float x3, float y3, int steps) {
    for (int i = 1; i <= steps; i++) {
        float t = (float)i / steps;
        float u = 1 - t;
        float x = u*u*u*x0 + 3*u*u*t*x1 + 3*u*t*t*x2 + t*t*t*x3;
        float y = u*u*u*y0 + 3*u*u*t*y1 + 3*u*t*t*y2 + t*t*t*y3;
        add_point(tx, ty, count, x, y, max);
    }
}

/* Helper: quadratic bezier interpolation */
static void quad_bezier(float *tx, float *ty, int *count, int max,
                        float x0, float y0, float x1, float y1, float x2, float y2, int steps) {
    for (int i = 1; i <= steps; i++) {
        float t = (float)i / steps;
        float u = 1 - t;
        float x = u*u*x0 + 2*u*t*x1 + t*t*x2;
        float y = u*u*y0 + 2*u*t*y1 + t*t*y2;
        add_point(tx, ty, count, x, y, max);
    }
}

/* Parse SVG path data string into points */
static int parse_svg_path(const char *d, float *tx, float *ty, int max) {
    int count = 0;
    float cx = 0, cy = 0;  /* Current position */
    float sx = 0, sy = 0;  /* Start of subpath (for Z command) */
    float lx = 0, ly = 0;  /* Last control point (for smooth curves) */
    char cmd = 0;
    const char *p = d;
    int bezier_steps = 10;
    
    while (*p) {
        p = skip_ws(p);
        if (!*p) break;
        
        /* Check for command letter */
        if ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z')) {
            cmd = *p++;
        }
        
        p = skip_ws(p);
        
        switch (cmd) {
            case 'M': { /* Move to (absolute) */
                float x, y;
                p = parse_float(p, &x);
                p = parse_float(p, &y);
                cx = x; cy = y;
                sx = cx; sy = cy;
                add_point(tx, ty, &count, cx, cy, max);
                cmd = 'L'; /* Subsequent coords are line-to */
                break;
            }
            case 'm': { /* Move to (relative) */
                float x, y;
                p = parse_float(p, &x);
                p = parse_float(p, &y);
                cx += x; cy += y;
                sx = cx; sy = cy;
                add_point(tx, ty, &count, cx, cy, max);
                cmd = 'l';
                break;
            }
            case 'L': { /* Line to (absolute) */
                float x, y;
                p = parse_float(p, &x);
                p = parse_float(p, &y);
                cx = x; cy = y;
                add_point(tx, ty, &count, cx, cy, max);
                break;
            }
            case 'l': { /* Line to (relative) */
                float x, y;
                p = parse_float(p, &x);
                p = parse_float(p, &y);
                cx += x; cy += y;
                add_point(tx, ty, &count, cx, cy, max);
                break;
            }
            case 'H': { /* Horizontal line (absolute) */
                float x;
                p = parse_float(p, &x);
                cx = x;
                add_point(tx, ty, &count, cx, cy, max);
                break;
            }
            case 'h': { /* Horizontal line (relative) */
                float x;
                p = parse_float(p, &x);
                cx += x;
                add_point(tx, ty, &count, cx, cy, max);
                break;
            }
            case 'V': { /* Vertical line (absolute) */
                float y;
                p = parse_float(p, &y);
                cy = y;
                add_point(tx, ty, &count, cx, cy, max);
                break;
            }
            case 'v': { /* Vertical line (relative) */
                float y;
                p = parse_float(p, &y);
                cy += y;
                add_point(tx, ty, &count, cx, cy, max);
                break;
            }
            case 'C': { /* Cubic bezier (absolute) */
                float x1, y1, x2, y2, x, y;
                p = parse_float(p, &x1);
                p = parse_float(p, &y1);
                p = parse_float(p, &x2);
                p = parse_float(p, &y2);
                p = parse_float(p, &x);
                p = parse_float(p, &y);
                cubic_bezier(tx, ty, &count, max, cx, cy, x1, y1, x2, y2, x, y, bezier_steps);
                lx = x2; ly = y2;
                cx = x; cy = y;
                break;
            }
            case 'c': { /* Cubic bezier (relative) */
                float x1, y1, x2, y2, x, y;
                p = parse_float(p, &x1);
                p = parse_float(p, &y1);
                p = parse_float(p, &x2);
                p = parse_float(p, &y2);
                p = parse_float(p, &x);
                p = parse_float(p, &y);
                cubic_bezier(tx, ty, &count, max, cx, cy, cx+x1, cy+y1, cx+x2, cy+y2, cx+x, cy+y, bezier_steps);
                lx = cx + x2; ly = cy + y2;
                cx += x; cy += y;
                break;
            }
            case 'S': { /* Smooth cubic bezier (absolute) */
                float x2, y2, x, y;
                p = parse_float(p, &x2);
                p = parse_float(p, &y2);
                p = parse_float(p, &x);
                p = parse_float(p, &y);
                float x1 = 2*cx - lx, y1 = 2*cy - ly;
                cubic_bezier(tx, ty, &count, max, cx, cy, x1, y1, x2, y2, x, y, bezier_steps);
                lx = x2; ly = y2;
                cx = x; cy = y;
                break;
            }
            case 's': { /* Smooth cubic bezier (relative) */
                float x2, y2, x, y;
                p = parse_float(p, &x2);
                p = parse_float(p, &y2);
                p = parse_float(p, &x);
                p = parse_float(p, &y);
                float x1 = 2*cx - lx, y1 = 2*cy - ly;
                cubic_bezier(tx, ty, &count, max, cx, cy, x1, y1, cx+x2, cy+y2, cx+x, cy+y, bezier_steps);
                lx = cx + x2; ly = cy + y2;
                cx += x; cy += y;
                break;
            }
            case 'Q': { /* Quadratic bezier (absolute) */
                float x1, y1, x, y;
                p = parse_float(p, &x1);
                p = parse_float(p, &y1);
                p = parse_float(p, &x);
                p = parse_float(p, &y);
                quad_bezier(tx, ty, &count, max, cx, cy, x1, y1, x, y, bezier_steps);
                lx = x1; ly = y1;
                cx = x; cy = y;
                break;
            }
            case 'q': { /* Quadratic bezier (relative) */
                float x1, y1, x, y;
                p = parse_float(p, &x1);
                p = parse_float(p, &y1);
                p = parse_float(p, &x);
                p = parse_float(p, &y);
                quad_bezier(tx, ty, &count, max, cx, cy, cx+x1, cy+y1, cx+x, cy+y, bezier_steps);
                lx = cx + x1; ly = cy + y1;
                cx += x; cy += y;
                break;
            }
            case 'Z':
            case 'z': { /* Close path */
                if (cx != sx || cy != sy) {
                    add_point(tx, ty, &count, sx, sy, max);
                }
                cx = sx; cy = sy;
                break;
            }
            default:
                p++; /* Skip unknown */
                break;
        }
    }
    return count;
}

/* Find and extract path data from SVG file content */
static const char *find_path_d(const char *svg, char *d_out, int max_len) {
    /* Look for <path with d=" attribute */
    const char *p = svg;
    while ((p = strstr(p, "<path")) != NULL) {
        const char *end = strchr(p, '>');
        if (!end) break;
        
        /* Find d=" within this path element */
        const char *d = strstr(p, " d=\"");
        if (!d || d > end) d = strstr(p, " d='");
        if (!d || d > end) { p = end; continue; }
        
        d += 4; /* Skip ' d="' */
        char quote = *(d - 1);
        const char *d_end = strchr(d, quote);
        if (!d_end) { p = end; continue; }
        
        int len = (int)(d_end - d);
        if (len >= max_len) len = max_len - 1;
        strncpy(d_out, d, len);
        d_out[len] = '\0';
        return d_out;
    }
    return NULL;
}

/* Load shape from an SVG file */
int load_svg_file(Vector2 *points, const char *filename, float cx, float cy, float scale, int max_points) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Could not open SVG file: %s\n", filename);
        return 0;
    }
    
    /* Read entire file */
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (fsize > 100000) fsize = 100000; /* Limit file size */
    
    char *svg = (char *)malloc((size_t)fsize + 1);
    if (!svg) { fclose(file); return 0; }
    
    fread(svg, 1, (size_t)fsize, file);
    svg[fsize] = '\0';
    fclose(file);
    
    /* Extract path data */
    char *path_d = (char *)malloc((size_t)fsize + 1);
    if (!path_d) { free(svg); return 0; }
    
    if (!find_path_d(svg, path_d, (int)fsize)) {
        printf("No path found in SVG: %s\n", filename);
        free(svg);
        free(path_d);
        return 0;
    }
    
    /* Parse path into temporary arrays */
    float *temp_x = (float *)malloc((size_t)max_points * sizeof(float));
    float *temp_y = (float *)malloc((size_t)max_points * sizeof(float));
    if (!temp_x || !temp_y) {
        free(svg); free(path_d);
        if (temp_x) free(temp_x);
        if (temp_y) free(temp_y);
        return 0;
    }
    
    int count = parse_svg_path(path_d, temp_x, temp_y, max_points);
    
    free(svg);
    free(path_d);
    
    if (count == 0) {
        free(temp_x);
        free(temp_y);
        return 0;
    }
    
    /* Find bounding box */
    float min_x = temp_x[0], max_x = temp_x[0];
    float min_y = temp_y[0], max_y = temp_y[0];
    for (int i = 1; i < count; i++) {
        if (temp_x[i] < min_x) min_x = temp_x[i];
        if (temp_x[i] > max_x) max_x = temp_x[i];
        if (temp_y[i] < min_y) min_y = temp_y[i];
        if (temp_y[i] > max_y) max_y = temp_y[i];
    }
    
    /* Center and scale */
    float orig_cx = (min_x + max_x) / 2.0f;
    float orig_cy = (min_y + max_y) / 2.0f;
    float width = max_x - min_x;
    float height = max_y - min_y;
    float max_dim = (width > height) ? width : height;
    float s = (max_dim > 0) ? (scale / max_dim) : 1.0f;
    
    for (int i = 0; i < count; i++) {
        points[i].x = cx + (temp_x[i] - orig_cx) * s;
        points[i].y = cy + (temp_y[i] - orig_cy) * s;
    }
    
    free(temp_x);
    free(temp_y);
    
    printf("Loaded %d points from SVG: %s\n", count, filename);
    return count;
}
