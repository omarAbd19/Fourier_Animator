#include "fourier.h"
#include <stdbool.h>

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

    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground((Color){12, 14, 24, 255});
            
            /* Calculate panel height dynamically */
            int panel_height = proceed ? 400 : 80;
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
                DrawText("Draw something to begin!", PANEL_X + PANEL_PADDING, y_pos, 14, (Color){120, 120, 140, 255});
            }

        was_drawing = is_drawing;

        EndDrawing();
    }
    
    if (complex_point) free(complex_point);
    if (epicycles) free(epicycles);
    
    CloseWindow();
    return 0;
}