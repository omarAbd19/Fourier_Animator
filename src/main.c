#include "raylib.h"
#include <stdlib.h>

int main (void)
{
    InitWindow(1000, 800, "Window");
    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(BLACK);
            DrawText("Window", 500, 400, 20, WHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}