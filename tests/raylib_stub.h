#ifndef RAYLIB_STUB_H
#define RAYLIB_STUB_H
/* Minimal type-check stub mirroring the raylib API used by win3wm.
   Only used for CI-type verification where raylib is unavailable. */
#include <stdbool.h>

typedef struct Vector2 { float x, y; } Vector2;
typedef struct Rectangle { float x, y, width, height; } Rectangle;
typedef struct Color { unsigned char r, g, b, a; } Color;
typedef struct Texture2D { unsigned int id; int width, height; } Texture2D;
typedef struct Font { int baseSize; } Font;
typedef struct Camera2D { struct { float x, y; } offset; struct { float x, y; } target; float rotation; float zoom; } Camera2D;

#define KEY_LEFT      263
#define KEY_RIGHT     262
#define KEY_UP        265
#define KEY_DOWN      264
#define KEY_ESCAPE    256
#define KEY_TAB       258
#define KEY_LEFT_ALT  342
#define KEY_RIGHT_ALT 344
#define KEY_F1        290
#define KEY_F2        291
#define KEY_LEFT_CONTROL 341
#define MOUSE_BUTTON_LEFT 0
#define MOUSE_BUTTON_RIGHT 2

void InitWindow(int w, int h, const char *title);
void CloseWindow(void);
bool WindowShouldClose(void);
void BeginDrawing(void);
void EndDrawing(void);
void ClearBackground(Color c);
void DrawRectangle(int x, int y, int w, int h, Color c);
void DrawRectangleLinesEx(Rectangle rec, int thick, Color c);
void DrawText(const char *text, int x, int y, int size, Color c);
void DrawLine(int x1, int y1, int x2, int y2, Color c);
void DrawFPS(int x, int y);
int MeasureText(const char *text, int size);
bool IsMouseButtonPressed(int button);
bool IsMouseButtonDown(int button);
bool IsMouseButtonReleased(int button);
Vector2 GetMousePosition(void);
Vector2 GetMouseDelta(void);
bool IsKeyPressed(int key);
int GetKeyPressed(void);
bool IsKeyDown(int key);
bool IsWindowResized(void);
int GetScreenWidth(void);
int GetScreenHeight(void);
void SetTargetFPS(int fps);
void TakeScreenshot(const char *name);
Color GetColor(unsigned int hex);
bool CheckCollisionPointRec(Vector2 p, Rectangle rec);
void SetTraceLogLevel(int level);
#endif
