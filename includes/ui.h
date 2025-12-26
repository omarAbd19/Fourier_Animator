/**
 * ui.h - User Interface Components
 * 
 * Reusable UI widgets for the Fourier Animator application.
 */

#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <stdbool.h>

/* ========== UI Layout Constants ========== */
#define PANEL_X         10
#define PANEL_Y         10
#define PANEL_WIDTH     280
#define PANEL_PADDING   15
#define ROW_HEIGHT      28

/* ========== UI Colors ========== */
#define COLOR_LABEL      (Color){180, 180, 200, 255}
#define COLOR_VALUE      (Color){255, 255, 255, 255}
#define COLOR_ACCENT     (Color){100, 150, 255, 255}
#define COLOR_PANEL_BG   (Color){25, 28, 40, 240}
#define COLOR_PANEL_BORDER (Color){60, 70, 100, 255}
#define COLOR_BACKGROUND (Color){12, 14, 24, 255}

#define COLOR_BTN_DEFAULT (Color){50, 60, 90, 255}
#define COLOR_BTN_HOVER   (Color){70, 90, 130, 255}
#define COLOR_BTN_PURPLE  (Color){70, 50, 90, 255}
#define COLOR_BTN_PURPLE_HOVER (Color){100, 70, 130, 255}

/* ========== UI Functions ========== */

/**
 * Draw a clickable button with hover effect.
 * 
 * @param x, y      Position of the button
 * @param w, h      Width and height
 * @param text      Button label
 * @param bg        Background color
 * @param hover     Hover color
 * @return          true if clicked this frame
 */
bool draw_button(int x, int y, int w, int h, const char *text, Color bg, Color hover);

/**
 * Draw a horizontal slider with label.
 * 
 * @param x, y      Position of the slider track
 * @param w         Width of the slider
 * @param value     Current value
 * @param min_val   Minimum value
 * @param max_val   Maximum value
 * @param label     Label text shown above slider
 * @return          New value (may be unchanged or dragged)
 */
float draw_slider(int x, int y, int w, float value, float min_val, float max_val, const char *label);

/**
 * Draw a horizontal separator line within the panel.
 * 
 * @param y         Y position of the line
 */
void draw_panel_separator(int y);

/* ========== Trace Color Presets ========== */
#define NUM_TRACE_COLORS 6

/**
 * Get the base color for trace rendering.
 * 
 * @param color_index   Index into color presets (0 to NUM_TRACE_COLORS-1)
 * @return              The base Color for that preset
 */
Color get_trace_color(int color_index);

/**
 * Draw color selection buttons.
 * 
 * @param x, y          Position to start drawing
 * @param current_index Current selected color index
 * @return              New color index (may be unchanged or clicked)
 */
int draw_color_picker(int x, int y, int current_index);

#endif /* UI_H */
