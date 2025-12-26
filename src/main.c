/**
 * main.c - Fourier Animator Application
 * 
 * A real-time visualization of the Discrete Fourier Transform using epicycles.
 * Draw any shape and watch it reconstructed by rotating circles!
 */

#define _CRT_SECURE_NO_WARNINGS
#include "fourier.h"
#include "shapes.h"
#include "ui.h"
#include <string.h>
#include <io.h>

/* ========== Shape Presets ========== */
static const char *SHAPE_NAMES[] = { "Circle", "Square", "Star", "Heart", "Infinity", "Spiral" };
static const int NUM_SHAPES = 6;

/* ========== Helper Functions ========== */

/**
 * Scan the shapes/ directory for .txt and .svg files.
 */
static void scan_shape_files(AppState *state) {
    struct _finddata_t fileinfo;
    
    /* Scan for .txt files */
    intptr_t handle = _findfirst("shapes/*.txt", &fileinfo);
    if (handle != -1) {
        do {
            if (!(fileinfo.attrib & _A_SUBDIR) && state->num_shape_files < MAX_SHAPE_FILES) {
                strncpy(state->shape_files[state->num_shape_files], fileinfo.name, MAX_FILENAME_LEN - 1);
                state->shape_files[state->num_shape_files][MAX_FILENAME_LEN - 1] = '\0';
                state->num_shape_files++;
            }
        } while (_findnext(handle, &fileinfo) == 0 && state->num_shape_files < MAX_SHAPE_FILES);
        _findclose(handle);
    }
    
    /* Scan for .svg files */
    handle = _findfirst("shapes/*.svg", &fileinfo);
    if (handle != -1) {
        do {
            if (!(fileinfo.attrib & _A_SUBDIR) && state->num_shape_files < MAX_SHAPE_FILES) {
                strncpy(state->shape_files[state->num_shape_files], fileinfo.name, MAX_FILENAME_LEN - 1);
                state->shape_files[state->num_shape_files][MAX_FILENAME_LEN - 1] = '\0';
                state->num_shape_files++;
            }
        } while (_findnext(handle, &fileinfo) == 0 && state->num_shape_files < MAX_SHAPE_FILES);
        _findclose(handle);
    }
}

/**
 * Load a shape file (SVG or TXT) into the drawing points.
 */
static bool load_file_into_state(AppState *state, const char *filepath, float center_x, float center_y) {
    int len = (int)strlen(filepath);
    
    if (len > 4 && (strcmp(&filepath[len-4], ".svg") == 0 || strcmp(&filepath[len-4], ".SVG") == 0)) {
        state->point_count = load_svg_file(state->drawing_points, filepath, center_x, center_y, 
                                           DEFAULT_LOAD_SCALE, DRAWING_POINTS_MAX);
    } else if (len > 4 && (strcmp(&filepath[len-4], ".txt") == 0 || strcmp(&filepath[len-4], ".TXT") == 0)) {
        state->point_count = load_shape_from_file(state->drawing_points, filepath, center_x, center_y, 
                                                  DEFAULT_LOAD_SCALE, DRAWING_POINTS_MAX);
    } else {
        return false;
    }
    
    return state->point_count > 0;
}

/**
 * Generate a preset shape and start animation.
 */
static void generate_preset_shape(AppState *state, int shape_index, float center_x, float center_y) {
    switch (shape_index) {
        case 0: state->point_count = generate_circle(state->drawing_points, center_x, center_y, DEFAULT_SHAPE_SIZE, DEFAULT_SHAPE_POINTS); break;
        case 1: state->point_count = generate_square(state->drawing_points, center_x, center_y, DEFAULT_SHAPE_SIZE * 2, DEFAULT_SHAPE_POINTS); break;
        case 2: state->point_count = generate_star(state->drawing_points, center_x, center_y, DEFAULT_SHAPE_SIZE, DEFAULT_SHAPE_SIZE * 0.4f, 5, DEFAULT_SHAPE_POINTS); break;
        case 3: state->point_count = generate_heart(state->drawing_points, center_x, center_y, DEFAULT_SHAPE_SIZE * 0.9f, DEFAULT_SHAPE_POINTS); break;
        case 4: state->point_count = generate_infinity(state->drawing_points, center_x, center_y, DEFAULT_SHAPE_SIZE * 1.5f, DEFAULT_SHAPE_POINTS); break;
        case 5: state->point_count = generate_spiral(state->drawing_points, center_x, center_y, DEFAULT_SHAPE_SIZE, DEFAULT_SHAPE_POINTS); break;
    }
}

/**
 * Handle file drag and drop.
 */
static void handle_drag_drop(AppState *state) {
    if (!IsFileDropped()) return;
    
    FilePathList dropped = LoadDroppedFiles();
    if (dropped.count > 0) {
        float center_x = WINDOW_WIDTH / 2.0f;
        float center_y = WINDOW_HEIGHT / 2.0f;
        
        /* Reset current animation */
        app_reset(state);
        state->restart_clicked = false; /* Don't block drawing */
        
        if (load_file_into_state(state, dropped.paths[0], center_x, center_y)) {
            app_start_animation(state);
        }
    }
    UnloadDroppedFiles(dropped);
}

/**
 * Draw the current drawing points (user's raw input).
 */
static void draw_input_path(AppState *state) {
    for (int i = 0; i < state->point_count - 1; i++) {
        DrawLine((int)state->drawing_points[i].x, (int)state->drawing_points[i].y, 
                 (int)state->drawing_points[i + 1].x, (int)state->drawing_points[i + 1].y, WHITE);
    }
}

/**
 * Draw the traced path during animation.
 */
static void draw_trace_path(AppState *state, Vector2 current_tip) {
    Color base_color = get_trace_color(state->color_index);
    
    /* Draw recorded trace points with gradient based on selected color */
    for (int i = 0; i < state->trace_count - 1; i++) {
        float alpha = (float)i / (state->trace_count > 1 ? state->trace_count : 1);
        /* Blend from darker to full color */
        Color trace_color = (Color){
            (unsigned char)(base_color.r * (0.3f + 0.7f * alpha)),
            (unsigned char)(base_color.g * (0.3f + 0.7f * alpha)),
            (unsigned char)(base_color.b * (0.3f + 0.7f * alpha)),
            255
        };
        DrawLineEx(state->trace_points[i], state->trace_points[i + 1], state->line_thickness, trace_color);
    }
    
    /* Connect last trace point to current tip with bright color */
    if (state->trace_count > 0) {
        Color tip_color = (Color){
            (unsigned char)(base_color.r > 200 ? 255 : base_color.r + 55),
            (unsigned char)(base_color.g > 200 ? 255 : base_color.g + 55),
            (unsigned char)(base_color.b > 200 ? 255 : base_color.b + 55),
            255
        };
        DrawLineEx(state->trace_points[state->trace_count - 1], current_tip, 
                   state->line_thickness, tip_color);
    }
}

/**
 * Update animation state (advance time, record trace).
 */
static void update_animation(AppState *state, Vector2 current_tip) {
    if (state->animation_done) return;
    
    state->frame_time += GetFrameTime() * state->speed;
    
    float step_time = 1.0f / 60.0f;
    if (state->frame_time >= step_time) {
        state->frame_time -= step_time;
        
        /* Record current position */
        state->trace_points[state->trace_count] = current_tip;
        state->trace_count++;
        
        state->current_k = (int)((state->t / (2 * PI)) * state->point_count) % state->point_count;
        
        /* Advance time */
        state->t += (2 * PI) / state->point_count;
        if (state->t >= 2 * PI) {
            state->animation_done = true;
            state->t = 2 * PI;
        }
    }
}

/**
 * Draw the animation panel (shown when animating).
 */
static void draw_animation_panel(AppState *state, int *y_pos) {
    /* Stats row */
    DrawText("N:", PANEL_X + PANEL_PADDING, *y_pos, 16, COLOR_LABEL);
    DrawText(TextFormat("%d", state->point_count), PANEL_X + 35, *y_pos, 16, COLOR_VALUE);
    
    DrawText("t:", PANEL_X + 100, *y_pos, 16, COLOR_LABEL);
    DrawText(TextFormat("%.2f", state->t), PANEL_X + 115, *y_pos, 16, COLOR_VALUE);
    
    float progress = (state->t / (2 * PI)) * 100;
    DrawText(TextFormat("%.0f%%", progress), PANEL_X + PANEL_WIDTH - 50, *y_pos, 16, 
             state->animation_done ? GREEN : YELLOW);
    *y_pos += ROW_HEIGHT + 5;
    
    draw_panel_separator(*y_pos);
    *y_pos += 12;
    
    /* Coefficient display */
    DrawText("Current Coefficient", PANEL_X + PANEL_PADDING, *y_pos, 14, COLOR_LABEL);
    *y_pos += 20;
    
    if (state->point_count > 0 && state->dft_result != NULL) {
        int k = state->current_k < state->point_count ? state->current_k : 0;
        float real_val = state->dft_result[k].real;
        float imag_val = state->dft_result[k].imag;
        
        DrawText(TextFormat("k = %d", k), PANEL_X + PANEL_PADDING, *y_pos, 16, COLOR_ACCENT);
        *y_pos += 22;
        
        char sign = imag_val >= 0 ? '+' : '-';
        float abs_imag = imag_val >= 0 ? imag_val : -imag_val;
        DrawText(TextFormat("X[k] = %.2f %c %.2fi", real_val, sign, abs_imag), 
                 PANEL_X + PANEL_PADDING, *y_pos, 16, COLOR_VALUE);
        *y_pos += 22;
        
        float amp = state->epicycles[k].amplitude;
        float phase = state->epicycles[k].phase;
        DrawText(TextFormat("|X| = %.2f   phi = %.2f", amp, phase), 
                 PANEL_X + PANEL_PADDING, *y_pos, 14, (Color){150, 150, 170, 255});
    }
    *y_pos += ROW_HEIGHT;
    
    draw_panel_separator(*y_pos);
    *y_pos += 15;
    
    /* Speed slider */
    state->speed = draw_slider(PANEL_X + PANEL_PADDING, *y_pos + 20, PANEL_WIDTH - 2 * PANEL_PADDING, 
                              state->speed, 0.1f, 5.0f, "Speed");
    *y_pos += 55;
    
    /* Line thickness slider */
    state->line_thickness = draw_slider(PANEL_X + PANEL_PADDING, *y_pos + 20, PANEL_WIDTH - 2 * PANEL_PADDING, 
                                       state->line_thickness, 0.5f, 8.0f, "Line Size");
    *y_pos += 55;
    
    /* Color picker */
    state->color_index = draw_color_picker(PANEL_X + PANEL_PADDING, *y_pos, state->color_index);
    *y_pos += 60;
    
    /* Restart button */
    if (draw_button(PANEL_X + PANEL_PADDING, *y_pos, PANEL_WIDTH - 2 * PANEL_PADDING, 35, 
                   "RESTART", (Color){60, 70, 100, 255}, (Color){80, 100, 140, 255})) {
        app_reset(state);
    }
}

/**
 * Draw the file picker panel (side panel).
 */
static void draw_file_picker(AppState *state, float center_x, float center_y, bool *mouse_on_panel) {
    if (!state->show_file_picker || state->num_shape_files <= 0) return;
    
    int picker_x = PANEL_X + PANEL_WIDTH + 10;
    int picker_y = PANEL_Y;
    int picker_w = 200;
    int picker_h = 40 + state->num_shape_files * 32;
    if (picker_h > 400) picker_h = 400;
    
    Rectangle picker_rect = { (float)picker_x, (float)picker_y, (float)picker_w, (float)picker_h };
    DrawRectangleRounded(picker_rect, 0.05f, 8, COLOR_PANEL_BG);
    DrawRectangleRoundedLines(picker_rect, 0.05f, 8, COLOR_PANEL_BORDER);
    
    if (CheckCollisionPointRec(GetMousePosition(), picker_rect)) {
        *mouse_on_panel = true;
    }
    
    DrawText("Select Shape File", picker_x + 10, picker_y + 10, 14, COLOR_ACCENT);
    
    int file_y = picker_y + 35;
    int visible_files = (picker_h - 45) / 32;
    
    /* Scroll with mouse wheel */
    if (CheckCollisionPointRec(GetMousePosition(), picker_rect)) {
        state->file_scroll -= (int)GetMouseWheelMove();
        if (state->file_scroll < 0) state->file_scroll = 0;
        if (state->file_scroll > state->num_shape_files - visible_files) 
            state->file_scroll = state->num_shape_files - visible_files;
        if (state->file_scroll < 0) state->file_scroll = 0;
    }
    
    for (int i = state->file_scroll; i < state->num_shape_files && i < state->file_scroll + visible_files; i++) {
        int btn_y = file_y + (i - state->file_scroll) * 32;
        
        /* Truncate filename if too long */
        char display_name[24];
        strncpy(display_name, state->shape_files[i], 23);
        display_name[23] = '\0';
        if (strlen(state->shape_files[i]) > 23) {
            display_name[20] = '.';
            display_name[21] = '.';
            display_name[22] = '.';
        }
        
        if (draw_button(picker_x + 10, btn_y, picker_w - 20, 28, display_name,
                       (Color){50, 50, 70, 255}, (Color){70, 80, 110, 255})) {
            char filepath[128];
            snprintf(filepath, sizeof(filepath), "shapes/%s", state->shape_files[i]);
            
            if (load_file_into_state(state, filepath, center_x, center_y)) {
                app_start_animation(state);
                state->show_file_picker = false;
            }
            state->restart_clicked = true;
        }
    }
    
    /* Scroll indicator */
    if (state->num_shape_files > visible_files) {
        int end_idx = state->file_scroll + visible_files;
        if (end_idx > state->num_shape_files) end_idx = state->num_shape_files;
        DrawText(TextFormat("[%d-%d of %d]", state->file_scroll + 1, end_idx, state->num_shape_files), 
                 picker_x + 10, picker_y + picker_h - 18, 10, (Color){80, 80, 100, 255});
    }
}

/**
 * Draw the input panel (shown when not animating).
 */
static void draw_input_panel(AppState *state, int *y_pos) {
    float center_x = WINDOW_WIDTH / 2.0f;
    float center_y = WINDOW_HEIGHT / 2.0f;
    
    DrawText("Draw, pick a shape, or drag SVG:", PANEL_X + PANEL_PADDING, *y_pos, 14, (Color){120, 120, 140, 255});
    *y_pos += 25;
    
    /* Shape preset buttons (2 columns) */
    int btn_width = (PANEL_WIDTH - 3 * PANEL_PADDING) / 2;
    int btn_height = 30;
    
    for (int i = 0; i < NUM_SHAPES; i++) {
        int col = i % 2;
        int row = i / 2;
        int btn_x = PANEL_X + PANEL_PADDING + col * (btn_width + PANEL_PADDING);
        int btn_y = *y_pos + row * (btn_height + 8);
        
        if (draw_button(btn_x, btn_y, btn_width, btn_height, SHAPE_NAMES[i],
                       COLOR_BTN_DEFAULT, COLOR_BTN_HOVER)) {
            generate_preset_shape(state, i, center_x, center_y);
            app_start_animation(state);
            state->restart_clicked = true;
        }
    }
    
    /* File browser section */
    *y_pos += 3 * (btn_height + 8) + 10;
    draw_panel_separator(*y_pos);
    *y_pos += 10;
    
    DrawText(TextFormat("Shape files (%d found):", state->num_shape_files), 
             PANEL_X + PANEL_PADDING, *y_pos, 12, (Color){100, 100, 120, 255});
    *y_pos += 18;
    
    const char *picker_text = state->show_file_picker ? "Hide Files" : "Browse Files...";
    if (draw_button(PANEL_X + PANEL_PADDING, *y_pos, PANEL_WIDTH - 2 * PANEL_PADDING, btn_height, 
                   picker_text, COLOR_BTN_PURPLE, COLOR_BTN_PURPLE_HOVER)) {
        state->show_file_picker = !state->show_file_picker;
        state->restart_clicked = true;
    }
    
    /* Draw drop hint at bottom */
    DrawText("Drag & drop SVG or TXT file anywhere", 
             WINDOW_WIDTH/2 - MeasureText("Drag & drop SVG or TXT file anywhere", 14)/2,
             WINDOW_HEIGHT - 40, 14, (Color){60, 60, 80, 255});
}

/* ========== Main Entry Point ========== */

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);
    
    /* Initialize application state */
    AppState state;
    app_state_init(&state);
    scan_shape_files(&state);
    
    /* Main loop */
    while (!WindowShouldClose()) {
        /* Calculate panel dimensions */
        int panel_height = state.proceed ? 460 : 290;  /* Increased for color picker */
        Rectangle panel_rect = { (float)PANEL_X, (float)PANEL_Y, (float)PANEL_WIDTH, (float)panel_height };
        bool mouse_on_panel = CheckCollisionPointRec(GetMousePosition(), panel_rect);
        
        /* Reset restart flag on mouse release */
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            state.restart_clicked = false;
        }
        
        /* Handle drag and drop */
        handle_drag_drop(&state);
        
        /* Handle mouse drawing */
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !mouse_on_panel && !state.restart_clicked && !state.proceed) {
            if (state.point_count < DRAWING_POINTS_MAX) {
                state.drawing_points[state.point_count] = GetMousePosition();
                state.point_count++;
                state.is_drawing = true;
            }
        } else if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            state.is_drawing = false;
        }
        
        /* Trigger animation when drawing ends */
        if (!state.is_drawing && state.was_drawing && !state.proceed && state.point_count > 0) {
            app_start_animation(&state);
        }
        
        /* ========== Rendering ========== */
        BeginDrawing();
        ClearBackground(COLOR_BACKGROUND);
        
        /* Draw input path */
        draw_input_path(&state);
        
        /* Animation rendering */
        if (state.proceed) {
            Vector2 tip = draw_epicycles(state.epicycles, state.point_count, state.t, state.line_thickness);
            draw_trace_path(&state, tip);
            update_animation(&state, tip);
        }
        
        /* ========== UI Panel ========== */
        DrawRectangleRounded(panel_rect, 0.05f, 8, COLOR_PANEL_BG);
        DrawRectangleRoundedLines(panel_rect, 0.05f, 8, COLOR_PANEL_BORDER);
        
        int y_pos = PANEL_Y + PANEL_PADDING;
        
        /* Title */
        DrawText("FOURIER VISUALIZER", PANEL_X + PANEL_PADDING, y_pos, 20, COLOR_ACCENT);
        y_pos += 30;
        draw_panel_separator(y_pos);
        y_pos += 15;
        
        if (state.proceed) {
            draw_animation_panel(&state, &y_pos);
        } else {
            draw_input_panel(&state, &y_pos);
            draw_file_picker(&state, WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, &mouse_on_panel);
        }
        
        state.was_drawing = state.is_drawing;
        EndDrawing();
    }
    
    /* Cleanup */
    if (state.dft_result) free(state.dft_result);
    if (state.epicycles) free(state.epicycles);
    CloseWindow();
    
    return 0;
}
