#define _CRT_SECURE_NO_WARNINGS
#include "fourier.h"
#include "shapes.h"
#include <stdbool.h>
#include <string.h>
#include <io.h>

#define MAX_SHAPE_FILES 20
#define MAX_FILENAME_LEN 64

/* UI Layout constants */
#define PANEL_X         10
#define PANEL_Y         10
#define PANEL_WIDTH     280
#define PANEL_PADDING   15
#define ROW_HEIGHT      28
#define LABEL_COLOR     (Color){180, 180, 200, 255}
#define VALUE_COLOR     (Color){255, 255, 255, 255}
#define ACCENT_COLOR    (Color){100, 150, 255, 255}
#define PANEL_BG        (Color){25, 28, 40, 240}
#define PANEL_BORDER    (Color){60, 70, 100, 255}

/* Button helper */
bool DrawButton(int x, int y, int w, int h, const char *text, Color bg, Color hover) {
    Rectangle btn = { (float)x, (float)y, (float)w, (float)h };
    bool is_hover = CheckCollisionPointRec(GetMousePosition(), btn);
    bool clicked = is_hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    
    DrawRectangleRounded(btn, 0.3f, 8, is_hover ? hover : bg);
    DrawRectangleRoundedLines(btn, 0.3f, 8, (Color){100, 120, 160, 255});
    
    int text_w = MeasureText(text, 16);
    DrawText(text, x + (w - text_w) / 2, y + (h - 16) / 2, 16, WHITE);
    
    return clicked;
}

/* Slider helper - returns new value */
float DrawSlider(int x, int y, int w, float value, float min_val, float max_val, const char *label) {
    /* Label */
    DrawText(label, x, y - 20, 14, LABEL_COLOR);
    DrawText(TextFormat("%.2fx", value), x + w - 45, y - 20, 14, VALUE_COLOR);
    
    /* Track */
    Rectangle track = { (float)x, (float)y + 5, (float)w, 8.0f };
    DrawRectangleRounded(track, 0.5f, 4, (Color){50, 55, 70, 255});
    
    /* Filled portion */
    float pct = (value - min_val) / (max_val - min_val);
    Rectangle filled = { (float)x, (float)y + 5, (float)(w * pct), 8.0f };
    DrawRectangleRounded(filled, 0.5f, 4, ACCENT_COLOR);
    
    /* Handle */
    int handle_x = x + (int)(w * pct);
    DrawCircle(handle_x, y + 9, 10, (Color){80, 130, 220, 255});
    DrawCircle(handle_x, y + 9, 6, WHITE);
    
    /* Drag logic */
    Rectangle drag_area = { (float)(x - 10), (float)y, (float)(w + 20), 20.0f };
    if (CheckCollisionPointRec(GetMousePosition(), drag_area) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float new_pct = (GetMousePosition().x - x) / w;
        if (new_pct < 0) new_pct = 0;
        if (new_pct > 1) new_pct = 1;
        return min_val + new_pct * (max_val - min_val);
    }
    return value;
}

int main (void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);

    Vector2 drawing_points[DRAWING_POINTS_MAX];
    int index = 0;

    bool is_drawing  = false;
    bool was_drawing = false;
    bool proceed     = false;

    complex_t *complex_point = NULL;
    epicycle_t *epicycles    = NULL;

    float t = 0;
    Vector2 trace_points[DRAWING_POINTS_MAX];
    int trace_index = 0;
    bool animation_done = false;
    
    float speed = 1.0f;
    int current_k = 0;
    float frame_time = 0.0f;
    bool restart_clicked = false;
    float line_thickness = 2.0f;
    
    /* Auto-draw shapes */
    const char *shape_names[] = { "Circle", "Square", "Star", "Heart", "Infinity", "Spiral" };
    int num_shapes = 6;
    int shape_points = 500;  /* Number of points for generated shapes */
    
    /* Shape file browser */
    char shape_files[MAX_SHAPE_FILES][MAX_FILENAME_LEN];
    int num_shape_files = 0;
    bool show_file_picker = false;
    int file_scroll = 0;
    
    /* Scan for shape files on startup using _findfirst/_findnext */
    {
        struct _finddata_t fileinfo;
        intptr_t handle = _findfirst("shapes/*.txt", &fileinfo);
        if (handle != -1) {
            do {
                if (!(fileinfo.attrib & _A_SUBDIR) && num_shape_files < MAX_SHAPE_FILES) {
                    strncpy(shape_files[num_shape_files], fileinfo.name, MAX_FILENAME_LEN - 1);
                    shape_files[num_shape_files][MAX_FILENAME_LEN - 1] = '\0';
                    num_shape_files++;
                }
            } while (_findnext(handle, &fileinfo) == 0 && num_shape_files < MAX_SHAPE_FILES);
            _findclose(handle);
        }
    }

    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground((Color){12, 14, 24, 255});
            
            /* Calculate panel height dynamically */
            int panel_height = proceed ? 400 : 290;
            Rectangle panel_rect = { (float)PANEL_X, (float)PANEL_Y, (float)PANEL_WIDTH, (float)panel_height };
            
            /* Check if mouse is over panel */
            bool mouse_on_panel = CheckCollisionPointRec(GetMousePosition(), panel_rect);
            
            /* Reset restart flag when mouse is released */
            if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                restart_clicked = false;
            }
            
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !mouse_on_panel && !restart_clicked && !proceed) {
                if (index < DRAWING_POINTS_MAX) {
                    drawing_points[index] = GetMousePosition();
                    index++;
                    is_drawing = true;
                }
            }
            else if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                is_drawing  = false;
            }
            
            for (int i = 0; i < index - 1; i++)
            {
                DrawLine((int)drawing_points[i].x, (int)drawing_points[i].y, (int)drawing_points[i + 1].x, (int)drawing_points[i + 1].y, WHITE);
            }

            if (is_drawing == false && was_drawing == true && proceed == false && index > 0) {
                complex_point = (complex_t *)malloc(sizeof(complex_t) * index);
                
                for (int i = 0; i < index; i++)
                {
                    complex_point[i].real = drawing_points[i].x;
                    complex_point[i].imag = drawing_points[i].y;
                } 
                complex_point = DFT(complex_point, index);
                epicycles = dft_to_epicycles(complex_point, index);
                proceed = true;
            }

            if (proceed) {
                /* Draw epicycles at current t - the tip IS where the line draws */
                Vector2 current_tip = draw_epicycles(epicycles, index, t, line_thickness);
                
                /* Draw the traced path (all previously recorded points) */
                for (int i = 0; i < trace_index - 1; i++)
                {
                    float alpha = (float)i / (trace_index > 1 ? trace_index : 1);
                    Color trace_color = (Color){255, (unsigned char)(50 + 150 * alpha), (unsigned char)(50 * alpha), 255};
                    DrawLineEx((Vector2){trace_points[i].x, trace_points[i].y}, 
                               (Vector2){trace_points[i + 1].x, trace_points[i + 1].y}, line_thickness, trace_color);
                }
                
                /* Draw line from last trace point to current epicycle tip */
                if (trace_index > 0) {
                    DrawLineEx(trace_points[trace_index - 1], current_tip, line_thickness, 
                              (Color){255, 200, 100, 255});
                }
                
                /* After drawing, advance time and record the current tip position */
                if (!animation_done) {
                    frame_time += GetFrameTime() * speed;
                    
                    float step_time = 1.0f / 60.0f;
                    if (frame_time >= step_time) {
                        frame_time -= step_time;
                        
                        /* Record the CURRENT tip position (matches what we just drew) */
                        trace_points[trace_index] = current_tip;
                        trace_index++;
                        
                        current_k = (int)((t / (2 * PI)) * index) % index;

                        /* NOW advance t for the next frame */
                        t += (2 * PI) / index;
                        if (t >= 2 * PI) {
                            animation_done = true;
                            t = 2 * PI;
                        }
                    }
                }
            }
            
            /* ========== INFO PANEL ========== */
            DrawRectangleRounded(panel_rect, 0.05f, 8, PANEL_BG);
            DrawRectangleRoundedLines(panel_rect, 0.05f, 8, PANEL_BORDER);
            
            int y_pos = PANEL_Y + PANEL_PADDING;
            
            /* Title */
            DrawText("FOURIER VISUALIZER", PANEL_X + PANEL_PADDING, y_pos, 20, ACCENT_COLOR);
            y_pos += 30;
            
            DrawLine(PANEL_X + PANEL_PADDING, y_pos, PANEL_X + PANEL_WIDTH - PANEL_PADDING, y_pos, PANEL_BORDER);
            y_pos += 15;
            
            if (proceed) {
                /* Stats row */
                DrawText("N:", PANEL_X + PANEL_PADDING, y_pos, 16, LABEL_COLOR);
                DrawText(TextFormat("%d", index), PANEL_X + 35, y_pos, 16, VALUE_COLOR);
                
                DrawText("t:", PANEL_X + 100, y_pos, 16, LABEL_COLOR);
                DrawText(TextFormat("%.2f", t), PANEL_X + 115, y_pos, 16, VALUE_COLOR);
                
                float progress = (t / (2 * PI)) * 100;
                DrawText(TextFormat("%.0f%%", progress), PANEL_X + PANEL_WIDTH - 50, y_pos, 16, 
                         animation_done ? GREEN : YELLOW);
                y_pos += ROW_HEIGHT + 5;
                
                DrawLine(PANEL_X + PANEL_PADDING, y_pos, PANEL_X + PANEL_WIDTH - PANEL_PADDING, y_pos, PANEL_BORDER);
                y_pos += 12;
                
                /* Complex value section */
                DrawText("Current Coefficient", PANEL_X + PANEL_PADDING, y_pos, 14, LABEL_COLOR);
                y_pos += 20;
                
                if (index > 0 && complex_point != NULL) {
                    int k_display = current_k < index ? current_k : 0;
                    float real_val = complex_point[k_display].real;
                    float imag_val = complex_point[k_display].imag;
                    
                    DrawText(TextFormat("k = %d", k_display), PANEL_X + PANEL_PADDING, y_pos, 16, ACCENT_COLOR);
                    y_pos += 22;
                    
                    char sign = imag_val >= 0 ? '+' : '-';
                    float abs_imag = imag_val >= 0 ? imag_val : -imag_val;
                    DrawText(TextFormat("X[k] = %.2f %c %.2fi", real_val, sign, abs_imag), 
                             PANEL_X + PANEL_PADDING, y_pos, 16, VALUE_COLOR);
                    y_pos += 22;
                    
                    float amp = epicycles[k_display].amplitude;
                    float phase = epicycles[k_display].phase;
                    DrawText(TextFormat("|X| = %.2f   phi = %.2f", amp, phase), 
                             PANEL_X + PANEL_PADDING, y_pos, 14, (Color){150, 150, 170, 255});
                }
                y_pos += ROW_HEIGHT;
                
                DrawLine(PANEL_X + PANEL_PADDING, y_pos, PANEL_X + PANEL_WIDTH - PANEL_PADDING, y_pos, PANEL_BORDER);
                y_pos += 15;
                
                /* Speed slider */
                speed = DrawSlider(PANEL_X + PANEL_PADDING, y_pos + 20, PANEL_WIDTH - 2 * PANEL_PADDING, 
                                   speed, 0.1f, 5.0f, "Speed");
                y_pos += 55;
                
                /* Line thickness slider */
                line_thickness = DrawSlider(PANEL_X + PANEL_PADDING, y_pos + 20, PANEL_WIDTH - 2 * PANEL_PADDING, 
                                            line_thickness, 0.5f, 8.0f, "Line Size");
                y_pos += 55;
                
                /* Restart button */
                if (DrawButton(PANEL_X + PANEL_PADDING, y_pos, PANEL_WIDTH - 2 * PANEL_PADDING, 35, 
                               "RESTART", (Color){60, 70, 100, 255}, (Color){80, 100, 140, 255})) {
                    if (complex_point) { free(complex_point); complex_point = NULL; }
                    if (epicycles) { free(epicycles); epicycles = NULL; }
                    index = 0;
                    t = 0;
                    trace_index = 0;
                    proceed = false;
                    animation_done = false;
                    current_k = 0;
                    was_drawing = false;
                    is_drawing = false;
                    frame_time = 0.0f;
                    restart_clicked = true;
                }
            } else {
                DrawText("Draw, pick a shape, or load file:", PANEL_X + PANEL_PADDING, y_pos, 14, (Color){120, 120, 140, 255});
                y_pos += 25;
                
                /* Auto-draw shape buttons - 2 columns */
                int btn_width = (PANEL_WIDTH - 3 * PANEL_PADDING) / 2;
                int btn_height = 30;
                float center_x = WINDOW_WIDTH / 2.0f;
                float center_y = WINDOW_HEIGHT / 2.0f;
                float shape_size = 250.0f;
                
                for (int i = 0; i < num_shapes; i++) {
                    int col = i % 2;
                    int row = i / 2;
                    int btn_x = PANEL_X + PANEL_PADDING + col * (btn_width + PANEL_PADDING);
                    int btn_y = y_pos + row * (btn_height + 8);
                    
                    if (DrawButton(btn_x, btn_y, btn_width, btn_height, shape_names[i],
                                   (Color){50, 60, 90, 255}, (Color){70, 90, 130, 255})) {
                        /* Generate the selected shape */
                        switch (i) {
                            case 0: index = generate_circle(drawing_points, center_x, center_y, shape_size, shape_points); break;
                            case 1: index = generate_square(drawing_points, center_x, center_y, shape_size * 2, shape_points); break;
                            case 2: index = generate_star(drawing_points, center_x, center_y, shape_size, shape_size * 0.4f, 5, shape_points); break;
                            case 3: index = generate_heart(drawing_points, center_x, center_y, shape_size * 0.9f, shape_points); break;
                            case 4: index = generate_infinity(drawing_points, center_x, center_y, shape_size * 1.5f, shape_points); break;
                            case 5: index = generate_spiral(drawing_points, center_x, center_y, shape_size, shape_points); break;
                        }
                        
                        /* Trigger DFT computation */
                        complex_point = (complex_t *)malloc(sizeof(complex_t) * index);
                        for (int j = 0; j < index; j++) {
                            complex_point[j].real = drawing_points[j].x;
                            complex_point[j].imag = drawing_points[j].y;
                        }
                        complex_point = DFT(complex_point, index);
                        epicycles = dft_to_epicycles(complex_point, index);
                        proceed = true;
                        restart_clicked = true;
                    }
                }
                
                /* Load from file button */
                y_pos += 3 * (btn_height + 8) + 10;
                DrawLine(PANEL_X + PANEL_PADDING, y_pos, PANEL_X + PANEL_WIDTH - PANEL_PADDING, y_pos, PANEL_BORDER);
                y_pos += 10;
                
                DrawText(TextFormat("Shape files (%d found):", num_shape_files), PANEL_X + PANEL_PADDING, y_pos, 12, (Color){100, 100, 120, 255});
                y_pos += 18;
                
                /* Toggle file picker button */
                const char *picker_text = show_file_picker ? "Hide Files" : "Browse Files...";
                if (DrawButton(PANEL_X + PANEL_PADDING, y_pos, PANEL_WIDTH - 2 * PANEL_PADDING, btn_height, 
                               picker_text, (Color){70, 50, 90, 255}, (Color){100, 70, 130, 255})) {
                    show_file_picker = !show_file_picker;
                    restart_clicked = true;
                }
                
                /* File picker panel (shown to the right when active) */
                if (show_file_picker && num_shape_files > 0) {
                    int picker_x = PANEL_X + PANEL_WIDTH + 10;
                    int picker_y = PANEL_Y;
                    int picker_w = 200;
                    int picker_h = 40 + num_shape_files * 32;
                    if (picker_h > 400) picker_h = 400;
                    
                    Rectangle picker_rect = { (float)picker_x, (float)picker_y, (float)picker_w, (float)picker_h };
                    DrawRectangleRounded(picker_rect, 0.05f, 8, PANEL_BG);
                    DrawRectangleRoundedLines(picker_rect, 0.05f, 8, PANEL_BORDER);
                    
                    /* Check if mouse is over picker panel too */
                    if (CheckCollisionPointRec(GetMousePosition(), picker_rect)) {
                        mouse_on_panel = true;
                    }
                    
                    DrawText("Select Shape File", picker_x + 10, picker_y + 10, 14, ACCENT_COLOR);
                    
                    int file_y = picker_y + 35;
                    int visible_files = (picker_h - 45) / 32;
                    
                    /* Scroll with mouse wheel when hovering picker */
                    if (CheckCollisionPointRec(GetMousePosition(), picker_rect)) {
                        file_scroll -= (int)GetMouseWheelMove();
                        if (file_scroll < 0) file_scroll = 0;
                        if (file_scroll > num_shape_files - visible_files) 
                            file_scroll = num_shape_files - visible_files;
                        if (file_scroll < 0) file_scroll = 0;
                    }
                    
                    for (int i = file_scroll; i < num_shape_files && i < file_scroll + visible_files; i++) {
                        int btn_y_pos = file_y + (i - file_scroll) * 32;
                        
                        /* Truncate filename if too long */
                        char display_name[24];
                        strncpy(display_name, shape_files[i], 23);
                        display_name[23] = '\0';
                        if (strlen(shape_files[i]) > 23) {
                            display_name[20] = '.';
                            display_name[21] = '.';
                            display_name[22] = '.';
                        }
                        
                        if (DrawButton(picker_x + 10, btn_y_pos, picker_w - 20, 28, display_name,
                                       (Color){50, 50, 70, 255}, (Color){70, 80, 110, 255})) {
                            char filepath[128];
                            snprintf(filepath, sizeof(filepath), "shapes/%s", shape_files[i]);
                            index = load_shape_from_file(drawing_points, filepath, center_x, center_y, 500.0f, DRAWING_POINTS_MAX);
                            if (index > 0) {
                                complex_point = (complex_t *)malloc(sizeof(complex_t) * index);
                                for (int j = 0; j < index; j++) {
                                    complex_point[j].real = drawing_points[j].x;
                                    complex_point[j].imag = drawing_points[j].y;
                                }
                                complex_point = DFT(complex_point, index);
                                epicycles = dft_to_epicycles(complex_point, index);
                                proceed = true;
                                show_file_picker = false;
                            }
                            restart_clicked = true;
                        }
                    }
                    
                    /* Scroll indicator */
                    if (num_shape_files > visible_files) {
                        DrawText(TextFormat("[%d-%d of %d]", file_scroll + 1, 
                                 file_scroll + visible_files > num_shape_files ? num_shape_files : file_scroll + visible_files,
                                 num_shape_files), 
                                 picker_x + 10, picker_y + picker_h - 18, 10, (Color){80, 80, 100, 255});
                    }
                }
            }

        was_drawing = is_drawing;

        EndDrawing();
    }
    
    if (complex_point) free(complex_point);
    if (epicycles) free(epicycles);
    
    CloseWindow();
    return 0;
}