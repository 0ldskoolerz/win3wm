/* win3wm — a minimalist Windows 3.x-style window manager built on raylib,
   with Lua-scriptable plugins. See README.md. */
#define _GNU_SOURCE

#include "wm.h"
#include "config.h"
#include "plugins.h"

#if __has_include(<raylib.h>)
#include <raylib.h>
#else
#include "../tests/raylib_stub.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---------- palette ---------- */
static Color C(const char *name, Color fallback) {
    struct { const char *n; Color c; } table[] = {
        {"teal",    {0,128,128,255}}, {"silver", {192,192,192,255}},
        {"navy",    {0,0,128,255}},   {"black",  {0,0,0,255}},
        {"white",   {255,255,255,255}},{"gray",   {128,128,128,255}},
        {"blue",    {0,0,255,255}},   {"red",    {255,0,0,255}},
        {"green",   {0,128,0,255}},
    };
    for (size_t i = 0; i < sizeof table / sizeof table[0]; i++)
        if (!strcmp(table[i].n, name)) return table[i].c;
    return fallback;
}

/* ---------- globals shared with plugin hooks ---------- */
static WmState g_wm;
static WmConfig g_cfg;
static PluginHost g_plugins;
static char g_status[128] = "win3wm listo";

/* modular status panel: sections in config order, plus plugin-defined ones */
#define PANEL_MAX 16
typedef struct PanelSlot {
    char name[24];
    char text[96];
} PanelSlot;
static PanelSlot g_panel[PANEL_MAX];
static int g_panel_count;

static int panel_find_or_add(const char *name) {
    for (int i = 0; i < g_panel_count; i++)
        if (!strcmp(g_panel[i].name, name)) return i;
    if (g_panel_count >= PANEL_MAX) return -1;
    snprintf(g_panel[g_panel_count].name, sizeof g_panel[0].name, "%s", name);
    g_panel[g_panel_count].text[0] = '\0';
    return g_panel_count++;
}

static void panel_set(const char *name, const char *text) {
    int i = panel_find_or_add(name);
    if (i >= 0) snprintf(g_panel[i].text, sizeof g_panel[0].text, "%s", text);
}

static int g_menu_open = 0;          /* File menu open? */
static int g_menu_sel = 0;
static int g_drag_id = -1;
static int g_drag_hit = WM_HIT_NONE;
static int g_last_x, g_last_y;

/* ---------- plugin hooks (called from Lua) ---------- */
void wm_hook_notify(const char *msg) {
    snprintf(g_status, sizeof g_status, "%s", msg);
}

void wm_hook_status(const char *module, const char *text) {
    panel_set(module, text);
}
int wm_hook_create_window(const char *t, int w, int h) {
    return wm_create_window(&g_wm, t, w, h);
}
bool wm_hook_close_window(int id) {
    return wm_close_window(&g_wm, id);
}
void wm_hook_focus(int id) { wm_focus(&g_wm, id); }
const char *wm_hook_window_title(int id) {
    WmWindow *w = wm_find(&g_wm, id);
    return w ? w->title : "";
}
void wm_hook_set_title(int id, const char *t) {
    WmWindow *w = wm_find(&g_wm, id);
    if (w) snprintf(w->title, WM_MAX_TITLE, "%s", t);
}
void wm_hook_focus_next(void) { wm_cycle_focus(&g_wm); }
int wm_hook_window_count(void) { return g_wm.count; }
int wm_hook_window_id(int idx) {
    if (idx < 0 || idx >= g_wm.count) return -1;
    return g_wm.windows[idx].id;
}

/* ---------- drawing helpers ---------- */
static void draw_bevel(int x, int y, int w, int h, bool raised, Color face) {
    DrawRectangle(x, y, w, h, face);
    Color hi = {255,255,255,255}, lo = C("gray", (Color){128,128,128,255});
    Color top = raised ? hi : lo, bot = raised ? lo : hi;
    DrawLine(x, y, x + w - 1, y, top);
    DrawLine(x, y, x, y + h - 1, top);
    DrawLine(x + w - 1, y, x + w - 1, y + h - 1, bot);
    DrawLine(x, y + h - 1, x + w - 1, y + h - 1, bot);
}

static void draw_window(const WmWindow *win) {
    Color face = C(g_cfg.theme_bg, (Color){192,192,192,255});
    Color fg = C(g_cfg.theme_fg, (Color){0,0,0,255});
    int x = win->box.x, y = win->box.y, w = win->box.w, h = win->box.h;
    draw_bevel(x, y, w, h, true, face);
    /* title bar */
    Color tb = win->focused ? C(g_cfg.theme_accent, (Color){0,0,128,255})
                            : (Color){128,128,128,255};
    DrawRectangle(x + 3, y + 3, w - 6, g_cfg.titlebar_h, tb);
    Color wtext = C("white", (Color){255,255,255,255});
    if (win->focused) DrawText(win->title, x + 8, y + 6, 10, wtext);
    else DrawText(win->title, x + 8, y + 6, 10, fg);
    /* buttons: maximize, minimize, close */
    int bw = 14, bx = x + w - 4 - bw;
    int by = y + 4;
    draw_bevel(bx, by, bw, bw, true, face);
    DrawText("^", bx + 4, by + 2, 8, fg);
    bx -= bw + 2;
    draw_bevel(bx, by, bw, bw, true, face);
    DrawText("_", bx + 3, by + 2, 8, fg);
    bx -= bw + 2;
    draw_bevel(bx, by, bw, bw, true, face);
    DrawText("x", bx + 4, by + 2, 8, fg);
    /* client area */
    DrawRectangle(x + 4, y + g_cfg.titlebar_h + 6, w - 8,
                  h - g_cfg.titlebar_h - 10, C("white", (Color){255,255,255,255}));
}

static void draw_tasks(int x, int y, int h, int max_w, Color face, Color fg) {
    for (int i = 0; i < g_wm.count; i++) {
        WmWindow *w = &g_wm.windows[i];
        if (x + 110 > x + max_w) break;
        draw_bevel(x, y, 110, h, !w->minimized && w->focused, face);
        const char *mark = w->minimized ? "[+] " : "";
        char label[80];
        snprintf(label, sizeof label, "%s%.10s", mark, w->title);
        DrawText(label, x + 4, y + 4, 9, fg);
        x += 114;
    }
}

static void draw_taskbar(int sw, int sh) {
    int th = g_cfg.taskbar_h;
    Color face = C(g_cfg.theme_bg, (Color){192,192,192,255});
    Color fg = C(g_cfg.theme_fg, (Color){0,0,0,255});
    draw_bevel(0, sh - th, sw, th, true, face);
    DrawText("win3wm", 6, sh - th + 6, 10, fg);
    int x = 70;
    int right = sw - 8;

    /* built-in panel sections declared in panel_modules (left to right) */
    char list[128];
    snprintf(list, sizeof list, "%s", g_cfg.panel_modules);
    char *save = NULL, *tok = strtok_r(list, ",", &save);
    while (tok) {
        while (*tok == ' ') tok++;
        char buf[96];
        if (!strcmp(tok, "tasks")) {
            draw_tasks(x, sh - th + 3, th - 6, sw / 2 - x, face, fg);
            x += g_wm.count * 114;
        } else if (!strcmp(tok, "clock")) {
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            strftime(buf, sizeof buf, "%H:%M", tm);
            DrawText(buf, right - MeasureText(buf, 9), sh - th + 7, 9, fg);
            right -= MeasureText(buf, 9) + 16;
        } else if (!strcmp(tok, "date")) {
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            strftime(buf, sizeof buf, "%d/%m/%Y", tm);
            DrawText(buf, right - MeasureText(buf, 9), sh - th + 7, 9, fg);
            right -= MeasureText(buf, 9) + 16;
        } else {
            /* plugin-defined slot: any module name registered via wm.status() */
            for (int i = 0; i < g_panel_count; i++) {
                if (!strcmp(g_panel[i].name, tok) && g_panel[i].text[0]) {
                    DrawText(g_panel[i].text, right - MeasureText(g_panel[i].text, 9),
                             sh - th + 7, 9, fg);
                    right -= MeasureText(g_panel[i].text, 9) + 16;
                }
            }
        }
        tok = strtok_r(NULL, ",", &save);
    }
    DrawText(g_status, right - MeasureText(g_status, 9), sh - th + 7, 9, fg);
}

static void draw_menu(int sw) {
    Color face = C(g_cfg.theme_bg, (Color){192,192,192,255});
    Color fg = C(g_cfg.theme_fg, (Color){0,0,0,255});
    draw_bevel(0, 0, 120, 66, true, face);
    const char *items[] = {"Nuevo", "Cascade", "Minimizar todo"};
    for (int i = 0; i < 3; i++) {
        if (i == g_menu_sel)
            DrawRectangle(2, 2 + i * 20, 116, 20, C(g_cfg.theme_accent, (Color){0,0,128,255}));
        Color c = (i == g_menu_sel) ? C("white", (Color){255,255,255,255}) : fg;
        DrawText(items[i], 8, 6 + i * 20, 10, c);
    }
    (void)sw;
}

static void cascade_all(void) {
    int n = 0;
    for (int i = 0; i < g_wm.count; i++) {
        WmWindow *w = &g_wm.windows[i];
        w->box.x = 24 + n * 26;
        w->box.y = 24 + n * 26;
        w->maximized = false;
        n = (n + 1) % 8;
    }
    wm_constrain(&g_wm);
}

/* ---------- input ---------- */
static void handle_input(void) {
    Vector2 m = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (g_menu_open && m.y < 66 && m.x < 120) {
            switch (g_menu_sel) {
            case 0: wm_create_window(&g_wm, "Nueva ventana",
                                     g_cfg.window_default_w, g_cfg.window_default_h); break;
            case 1: cascade_all(); break;
            case 2:
                for (int i = 0; i < g_wm.count; i++)
                    if (!g_wm.windows[i].minimized)
                        wm_toggle_minimize(&g_wm, g_wm.windows[i].id);
                break;
            }
            g_menu_open = 0;
            return;
        }
        g_menu_open = 0;
        /* taskbar: "start" button opens the menu */
        if (m.y >= g_wm.screen_h) {
            if (m.x < 70) { g_menu_open = !g_menu_open; return; }
            /* taskbar window buttons */
            int x = 70;
            for (int i = 0; i < g_wm.count; i++) {
                if (m.x >= x && m.x <= x + 110 && m.x < g_cfg.screen_w - 140) {
                    int id = g_wm.windows[i].id;
                    WmWindow *w = wm_find(&g_wm, id);
                    if (w->minimized || !w->focused) wm_restore(&g_wm, id);
                    else wm_toggle_minimize(&g_wm, id);
                    return;
                }
                x += 114;
            }
            return;
        }
        WmWindow *w = wm_window_at(&g_wm, (int)m.x, (int)m.y);
        if (w) {
            int wid = w->id;
            char title[WM_MAX_TITLE];
            snprintf(title, sizeof title, "%s", w->title);
            wm_focus(&g_wm, wid);
            WmHit hit = wm_hit_test(w, (int)m.x, (int)m.y);
            switch (hit) {
            case WM_HIT_CLOSE:
                wm_close_window(&g_wm, wid);
                plugins_call(&g_plugins, "on_window_closed", "d", wid);
                break;
            case WM_HIT_MIN:  wm_toggle_minimize(&g_wm, wid); break;
            case WM_HIT_MAX:  wm_toggle_maximize(&g_wm, wid); break;
            case WM_HIT_TITLE: g_drag_id = wid; g_drag_hit = WM_HIT_TITLE; break;
            default: g_drag_id = wid; g_drag_hit = hit; break;
            }
            if (hit != WM_HIT_CLOSE)
                plugins_call(&g_plugins, "on_window_focused", "ds", wid, title);
        }
        g_last_x = (int)m.x; g_last_y = (int)m.y;
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && g_drag_id >= 0) {
        Vector2 d = GetMouseDelta();
        if (g_drag_hit == WM_HIT_TITLE)
            wm_move_window(&g_wm, g_drag_id, (int)d.x, (int)d.y);
        else if (g_drag_hit >= WM_HIT_EDGE_L)
            wm_resize_window(&g_wm, g_drag_id, g_drag_hit, (int)d.x, (int)d.y);
        g_last_x = (int)m.x; g_last_y = (int)m.y;
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) { g_drag_id = -1; g_drag_hit = WM_HIT_NONE; }

    /* keyboard */
    if (IsKeyPressed(KEY_TAB) && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
        wm_cycle_focus(&g_wm);
    if (IsKeyPressed(KEY_F1)) wm_create_window(&g_wm, "Ayuda", 300, 160);
    if (IsKeyPressed(KEY_ESCAPE)) g_menu_open = 0;
    if (g_menu_open) {
        if (IsKeyPressed(KEY_DOWN)) g_menu_sel = (g_menu_sel + 1) % 3;
        if (IsKeyPressed(KEY_UP)) g_menu_sel = (g_menu_sel + 2) % 3;
    }
}

/* ---------- main ---------- */
int main(int argc, char **argv) {
    const char *cfg_path = "config/win3wm.conf";
    if (argc > 1) cfg_path = argv[1];
    if (!cfg_load(&g_cfg, cfg_path)) cfg_defaults(&g_cfg);

    int sw = g_cfg.screen_w, sh = g_cfg.screen_h;
    InitWindow(sw, sh, "win3wm — Windows 3.x style window manager");
    SetTargetFPS(60);

    wm_init(&g_wm, sw, g_cfg.show_taskbar ? sh - g_cfg.taskbar_h : sh);
    plugins_init(&g_plugins);
    if (g_plugins.lua_available) {
        char list[128];
        snprintf(list, sizeof list, "%s", g_cfg.plugins);
        char *save = NULL, *tok = strtok_r(list, ",", &save);
        while (tok) {
            plugins_load(&g_plugins, tok);
            tok = strtok_r(NULL, ",", &save);
        }
    }
    plugins_call(&g_plugins, "on_start", "");

    while (!WindowShouldClose()) {
        handle_input();
        plugins_call(&g_plugins, "on_tick", "");
        BeginDrawing();
        ClearBackground(C(g_cfg.wallpaper, (Color){0,128,128,255}));
        /* draw z order: lowest first */
        for (int z = 0; z <= g_wm.count; z++)
            for (int i = 0; i < g_wm.count; i++)
                if (!g_wm.windows[i].minimized && g_wm.windows[i].z == z)
                    draw_window(&g_wm.windows[i]);
        if (g_cfg.show_taskbar) draw_taskbar(sw, sh);
        if (g_menu_open) draw_menu(sw);
        EndDrawing();
    }
    plugins_call(&g_plugins, "on_stop", "");
    CloseWindow();
    return 0;
}
