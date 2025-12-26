/**
 * ui.c - User Interface Components Implementation
 */

#include "ui.h"

bool draw_button(int x, int y, int w, int h, const char *text, Color bg, Color hover) {
    Rectangle btn = { (float)x, (float)y, (float)w, (float)h };
    bool is_hover = CheckCollisionPointRec(GetMousePosition(), btn);
    bool clicked = is_hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    
    DrawRectangleRounded(btn, 0.3f, 8, is_hover ? hover : bg);
    DrawRectangleRoundedLines(btn, 0.3f, 8, (Color){100, 120, 160, 255});
    
    int text_w = MeasureText(text, 16);
    DrawText(text, x + (w - text_w) / 2, y + (h - 16) / 2, 16, WHITE);
    
    return clicked;
}

float draw_slider(int x, int y, int w, float value, float min_val, float max_val, const char *label) {
    /* Label */
    DrawText(label, x, y - 20, 14, COLOR_LABEL);
    DrawText(TextFormat("%.2fx", value), x + w - 45, y - 20, 14, COLOR_VALUE);
    
    /* Track */
    Rectangle track = { (float)x, (float)y + 5, (float)w, 8.0f };
    DrawRectangleRounded(track, 0.5f, 4, (Color){50, 55, 70, 255});
    
    /* Filled portion */
    float pct = (value - min_val) / (max_val - min_val);
    Rectangle filled = { (float)x, (float)y + 5, (float)(w * pct), 8.0f };
    DrawRectangleRounded(filled, 0.5f, 4, COLOR_ACCENT);
    
    /* Handle */
    int handle_x = x + (int)(w * pct);
    DrawCircle(handle_x, y + 9, 10, (Color){80, 130, 220, 255});
    DrawCircle(handle_x, y + 9, 6, WHITE);
    
    /* Drag logic */
    Rectangle drag_area = { (float)(x - 10), (float)y, (float)(w + 20), 20.0f };
    if (CheckCollisionPointRec(GetMousePosition(), drag_area) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float new_pct = (GetMousePosition().x - (float)x) / (float)w;
        if (new_pct < 0) new_pct = 0;
        if (new_pct > 1) new_pct = 1;
        return min_val + new_pct * (max_val - min_val);
    }
    return value;
}

void draw_panel_separator(int y) {
    DrawLine(PANEL_X + PANEL_PADDING, y, PANEL_X + PANEL_WIDTH - PANEL_PADDING, y, COLOR_PANEL_BORDER);
}

/* ========== Trace Color Presets ========== */

/* Base colors for each preset */
static const Color TRACE_COLORS[NUM_TRACE_COLORS] = {
    {255, 80, 50, 255},    /* Fire - red/orange */
    {50, 150, 255, 255},   /* Ocean - blue */
    {100, 255, 100, 255},  /* Lime - green */
    {200, 100, 255, 255},  /* Purple */
    {255, 200, 50, 255},   /* Gold - yellow */
    {50, 255, 220, 255}    /* Cyan */
};

Color get_trace_color(int color_index) {
    if (color_index < 0 || color_index >= NUM_TRACE_COLORS) {
        color_index = 0;
    }
    return TRACE_COLORS[color_index];
}

int draw_color_picker(int x, int y, int current_index) {
    DrawText("Line Color", x, y, 14, COLOR_LABEL);
    y += 20;
    
    int btn_size = 32;
    int spacing = 8;
    int new_index = current_index;
    
    for (int i = 0; i < NUM_TRACE_COLORS; i++) {
        int btn_x = x + i * (btn_size + spacing);
        Rectangle btn = { (float)btn_x, (float)y, (float)btn_size, (float)btn_size };
        
        bool is_hover = CheckCollisionPointRec(GetMousePosition(), btn);
        bool is_selected = (i == current_index);
        
        /* Draw color swatch */
        DrawRectangleRounded(btn, 0.3f, 4, TRACE_COLORS[i]);
        
        /* Selection indicator */
        if (is_selected) {
            DrawRectangleRoundedLines(btn, 0.3f, 4, WHITE);
            /* Inner border for emphasis */
            Rectangle inner = { btn.x + 2, btn.y + 2, btn.width - 4, btn.height - 4 };
            DrawRectangleRoundedLines(inner, 0.3f, 4, (Color){0, 0, 0, 150});
        } else if (is_hover) {
            DrawRectangleRoundedLines(btn, 0.3f, 4, (Color){200, 200, 200, 200});
        }
        
        /* Click detection */
        if (is_hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            new_index = i;
        }
    }
    
    return new_index;
}