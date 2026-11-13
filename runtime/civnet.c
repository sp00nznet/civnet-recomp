/* civnet.c - what CivNet needs beyond pcrecomp's Win16 host.
 *
 * - WinG: CivNet loads WING.DLL with LoadLibrary and takes every function with
 *   GetProcAddress, so the host's WinG shims (reached through imports in
 *   Space Bucks) are handed out here as host thunks (RecompGame.proc_address).
 * - INT 2Fh AX=1689h, Windows' "kernel idle": the game makes it while it waits.
 */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "recomp_game.h"
#include "cpu.h"

void WING_WINGCREATEDC(CPU *cpu);
void WING_WINGRECOMMENDDIBFORMAT(CPU *cpu);
void WING_WINGCREATEBITMAP(CPU *cpu);
void WING_WINGSTRETCHBLT(CPU *cpu);
void WING_WINGBITBLT(CPU *cpu);
void WING_WINGGETDIBPOINTER(CPU *cpu);
void WING_WINGSETDIBCOLORTABLE(CPU *cpu);
void WING_WINGGETDIBCOLORTABLE(CPU *cpu);

/* Not in the host yet. CivNet asks for both; 0 (no palette, no brush) is
 * what WinG returns on failure. PASCAL: the callee pops. */
static void wing_halftone_palette(CPU *cpu) { cpu->ax = 0; cpu->sp += 4; }
static void wing_halftone_brush(CPU *cpu) { cpu->ax = 0; cpu->sp += 4 + 8; }  /* HDC, COLORREF, type */

static const struct { const char *name; void (*fn)(CPU *); } wing[] = {
    { "WinGCreateDC", WING_WINGCREATEDC },
    { "WinGRecommendDIBFormat", WING_WINGRECOMMENDDIBFORMAT },
    { "WinGCreateBitmap", WING_WINGCREATEBITMAP },
    { "WinGStretchBlt", WING_WINGSTRETCHBLT },
    { "WinGBitBlt", WING_WINGBITBLT },
    { "WinGGetDIBPointer", WING_WINGGETDIBPOINTER },
    { "WinGSetDIBColorTable", WING_WINGSETDIBCOLORTABLE },
    { "WinGGetDIBColorTable", WING_WINGGETDIBCOLORTABLE },
    { "WinGCreateHalftonePalette", wing_halftone_palette },
    { "WinGCreateHalftoneBrush", wing_halftone_brush },
};

static void wing_thunk(CPU *cpu, uint16_t i) { wing[i].fn(cpu); }

static int proc_address(uint16_t hinst, const char *name, int ordinal, uint16_t *seg, uint16_t *off)
{
    (void)hinst; (void)ordinal;
    if (!name) return 0;
    for (unsigned i = 0; i < sizeof wing / sizeof wing[0]; i++)
        if (!_stricmp(name, wing[i].name)) {
            uint32_t p = recomp_host_thunk(wing_thunk, (uint16_t)i);
            *seg = (uint16_t)(p >> 16);
            *off = (uint16_t)p;
            return p != 0;
        }
    return 0;
}

/* RECOMP_IRQ (CMakeLists.txt): lifted code calls this at function entries and
 * loop back-edges. The multimedia timer's callbacks are delivered here, as
 * interrupts were: the movie player waits for them without pumping messages. */
void recomp_mmtimer_poll(void);
int g_recomp_tick_budget = 1;
void recomp_tick(CPU *cpu)
{
    (void)cpu;
    g_recomp_tick_budget = 2000;
    recomp_mmtimer_poll();
}

void int_handler(CPU *cpu, int vec)
{
    if (vec == 0x2F && (cpu->ax == 0x1689 || cpu->ax == 0x1680)) { Sleep(1); return; }
    static int told[256];
    if (!told[vec & 255]++) fprintf(stderr, "[int] unhandled INT 0x%02X ax=%04X\n", vec, cpu->ax);
}

/* ---- CIVNET_SHOT="S:file.bmp,...": the game's window at S seconds ---------- */

static struct { double at; char path[MAX_PATH]; } g_shots[64];
static int g_nshots;

/* A child window's surface, copied into the capture at its place. EnumChildWindows
 * walks parents before their children, so nested children land on top. */
static HDC g_comp_dc;
static HWND g_comp_top;
static BOOL CALLBACK comp_child(HWND c, LPARAM lp)
{
    (void)lp;
    if (!IsWindowVisible(c)) return TRUE;
    RECT r;
    GetClientRect(c, &r);
    POINT o = { 0, 0 };
    MapWindowPoints(c, g_comp_top, &o, 1);
    HDC dc = GetDC(c);
    BitBlt(g_comp_dc, o.x, o.y, r.right, r.bottom, dc, 0, 0, SRCCOPY);
    ReleaseDC(c, dc);
    return TRUE;
}

static void shot_window(HWND w, const char *path)
{
    RECT r;
    if (!GetClientRect(w, &r) || !r.right || !r.bottom) return;
    HDC wdc = GetDC(w), mdc = CreateCompatibleDC(wdc);
    BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), r.right, r.bottom, 1, 24, BI_RGB } };
    void *bits;
    HBITMAP bm = CreateDIBSection(wdc, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
    HGDIOBJ old = SelectObject(mdc, bm);
    BitBlt(mdc, 0, 0, r.right, r.bottom, wdc, 0, 0, SRCCOPY);   /* the window's own surface */
    g_comp_dc = mdc; g_comp_top = w;
    EnumChildWindows(w, comp_child, 0);                         /* and each child's, on top */
    GdiFlush();
    DWORD stride = ((DWORD)r.right * 3 + 3) & ~3u, size = stride * r.bottom;
    BITMAPFILEHEADER fh = { 0x4D42, sizeof fh + sizeof bi.bmiHeader + size, 0, 0, sizeof fh + sizeof bi.bmiHeader };
    FILE *f = fopen(path, "wb");
    if (f) { fwrite(&fh, sizeof fh, 1, f); fwrite(&bi.bmiHeader, sizeof bi.bmiHeader, 1, f); fwrite(bits, size, 1, f); fclose(f); }
    char t[64] = "";
    { WCHAR wt[64]; InternalGetWindowText(w, wt, 64); WideCharToMultiByte(CP_ACP, 0, wt, -1, t, sizeof t, NULL, NULL); }
    fprintf(stderr, "[shot] \"%s\" %ldx%ld -> %s\n", t, r.right, r.bottom, path);
    SelectObject(mdc, old);
    DeleteObject(bm);
    DeleteDC(mdc);
    ReleaseDC(w, wdc);
}

/* Every visible top-level window of this process: file.bmp, file-1.bmp, ... */
static const char *g_shot_path;
static int g_shot_n;
static BOOL CALLBACK shot_each(HWND w, LPARAM lp)
{
    (void)lp;
    DWORD pid;
    GetWindowThreadProcessId(w, &pid);
    if (pid != GetCurrentProcessId() || !IsWindowVisible(w)) return TRUE;
    char path[MAX_PATH];
    if (g_shot_n == 0) snprintf(path, sizeof path, "%s", g_shot_path);
    else {
        const char *dot = strrchr(g_shot_path, '.');
        int stem = dot ? (int)(dot - g_shot_path) : (int)strlen(g_shot_path);
        snprintf(path, sizeof path, "%.*s-%d%s", stem, g_shot_path, g_shot_n, dot ? dot : "");
    }
    g_shot_n++;
    shot_window(w, path);
    return TRUE;
}

static void shot(const char *path)
{
    g_shot_path = path;
    g_shot_n = 0;
    EnumWindows(shot_each, 0);
}

/* CIVNET_KEYS="S:key,S:click:x:y,..." posts input to the game's own windows:
 * a key (esc enter space tab up down left right, a letter or digit) to the
 * main window, a click at client coordinates of the main window to whichever
 * child is there. Merged with the shots, by time. */
static HWND main_window(void)
{
    HWND w = FindWindowA(NULL, "Sid Meier's CivNet");
    return w;
}

static HWND g_pop;
/* The dialog on top: a separate top-level window, or (real_windows) the
 * topmost visible child of the main frame. */
static void find_dialog(HWND top);
static BOOL CALLBACK find_pop(HWND w, LPARAM main)
{
    DWORD pid;
    GetWindowThreadProcessId(w, &pid);
    if (pid == GetCurrentProcessId() && IsWindowVisible(w) && w != (HWND)main) { g_pop = w; return FALSE; }
    return TRUE;
}

static void find_dialog(HWND top)
{
    g_pop = NULL;
    EnumWindows(find_pop, (LPARAM)top);
    for (HWND c = GetWindow(top, GW_CHILD); !g_pop && c; c = GetWindow(c, GW_HWNDNEXT)) {
        RECT r;
        GetClientRect(c, &r);
        if (IsWindowVisible(c) && r.right < 640) g_pop = c;   /* not the full-screen backdrop */
    }
}

static void act(const char *what)
{
    if (!strcmp(what, "ring")) { void dump_fn_ring(int n); dump_fn_ring(60); return; }
    if (!strcmp(what, "snap")) { extern volatile int recomp_snap_req; recomp_snap_req = 1; return; }
    if (!strncmp(what, "peek:", 5)) {   /* peek:SEG:OFF:N (hex) - guest bytes */
        extern CPU *g_cpu; unsigned sg, of, n;
        if (sscanf(what + 5, "%x:%x:%x", &sg, &of, &n) == 3) {
            fprintf(stderr, "[peek] %04X:%04X", sg, of);
            for (unsigned i = 0; i < n; i++) fprintf(stderr, "%s%02X", i % 32 ? " " : "\n  ", mem_read8(g_cpu, (uint16_t)sg, (uint16_t)(of + i)));
            fprintf(stderr, "\n");
        }
        return;
    }
    if (!strcmp(what, "quit")) { fflush(stderr); exit(0); }   /* runs the atexit reports */
    if (!strcmp(what, "repaint")) { RedrawWindow(main_window(), NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN); fprintf(stderr, "[keys] repaint\n"); return; }
    HWND top = main_window();
    /* item:N clicks the middle of the Nth (from 1) visible child of the
     * pop-up: a menu's entries, a dialog's buttons. */
    if (!strncmp(what, "item:", 5)) {
        g_pop = NULL;
        EnumWindows(find_pop, (LPARAM)top);
        if (!g_pop) { fprintf(stderr, "[keys] no pop-up\n"); return; }
        int n = atoi(what + 5), k = 0;
        for (HWND c = GetWindow(g_pop, GW_CHILD); c; c = GetWindow(c, GW_HWNDNEXT)) {
            if (!IsWindowVisible(c) || ++k != n) continue;
            RECT r;
            GetClientRect(c, &r);
            LPARAM lp = MAKELPARAM(r.right / 2, r.bottom / 2);
            PostMessageA(c, WM_MOUSEMOVE, 0, lp);
            PostMessageA(c, WM_LBUTTONDOWN, MK_LBUTTON, lp);
            PostMessageA(c, WM_LBUTTONUP, 0, lp);
            fprintf(stderr, "[keys] item %d -> %p (%ldx%ld)\n", n, (void *)c, r.right, r.bottom);
            return;
        }
        fprintf(stderr, "[keys] pop-up has %d items\n", k);
        return;
    }
    /* pop:x:y clicks at client x,y of the other visible top-level window (a
     * pop-up bar or dialog) instead of the main one. */
    if (!strncmp(what, "pop:", 4)) {
        static char buf[64];
        g_pop = NULL;
        EnumWindows(find_pop, (LPARAM)top);
        if (!g_pop) { fprintf(stderr, "[keys] no pop-up\n"); return; }
        top = g_pop;
        snprintf(buf, sizeof buf, "click:%s", what + 4);
        what = buf;
    }
    if (!top) { fprintf(stderr, "[keys] no main window for %s\n", what); return; }
    if (!strncmp(what, "click:", 6)) {
        int x = 0, y = 0;
        sscanf(what + 6, "%d:%d", &x, &y);
        POINT p = { x, y };
        ClientToScreen(top, &p);
        HWND w = top, c;
        for (;;) {                                   /* the deepest child there */
            POINT q = p;
            ScreenToClient(w, &q);
            c = RealChildWindowFromPoint(w, q);
            if (!c || c == w) break;
            w = c;
        }
        POINT q = p;
        ScreenToClient(w, &q);
        LPARAM lp = MAKELPARAM(q.x, q.y);
        PostMessageA(w, WM_MOUSEMOVE, 0, lp);
        PostMessageA(w, WM_LBUTTONDOWN, MK_LBUTTON, lp);
        PostMessageA(w, WM_LBUTTONUP, 0, lp);
        fprintf(stderr, "[keys] click %d,%d -> window %p at %ld,%ld\n", x, y, (void *)w, q.x, q.y);
        return;
    }
    static const struct { const char *n; int vk; } t[] = {
        {"esc",VK_ESCAPE},{"enter",VK_RETURN},{"space",VK_SPACE},{"tab",VK_TAB},
        {"up",VK_UP},{"down",VK_DOWN},{"left",VK_LEFT},{"right",VK_RIGHT}};
    int vk = 0;
    for (unsigned i = 0; i < sizeof t / sizeof t[0]; i++) if (!_stricmp(what, t[i].n)) vk = t[i].vk;
    if (!vk && what[0] && !what[1]) vk = (unsigned char)(what[0] >= 'a' && what[0] <= 'z' ? what[0] - 32 : what[0]);
    if (!vk) return;
    LPARAM lp = 1 | (MapVirtualKeyA(vk, 0) << 16);
    /* To the pop-up when one is open (a menu, a dialog), else the main window. */
    find_dialog(top);
    HWND f = g_pop ? g_pop : top;
    PostMessageA(f, WM_KEYDOWN, vk, lp);
    if (vk >= '0' && vk <= 'Z') PostMessageA(f, WM_CHAR, what[0], lp);
    PostMessageA(f, WM_KEYUP, vk, lp | 0xC0000000);
    fprintf(stderr, "[keys] %s\n", what);
}

static DWORD WINAPI shot_thread(LPVOID p)
{
    (void)p;
    DWORD t0 = GetTickCount();
    for (int i = 0; i < g_nshots; i++) {
        DWORD at = t0 + (DWORD)(g_shots[i].at * 1000), now = GetTickCount();
        if ((int)(at - now) > 0) Sleep(at - now);
        if (g_shots[i].path[0] == '!') act(g_shots[i].path + 1);
        else shot(g_shots[i].path);
    }
    return 0;
}

static int by_time(const void *a, const void *b)
{
    double d = *(const double *)a - *(const double *)b;
    return d < 0 ? -1 : d > 0;
}

static void parse_due(const char *s, int action)
{
    while (s && *s && g_nshots < 64) {
        char *end;
        g_shots[g_nshots].at = strtod(s, &end);
        if (*end != ':') break;
        s = end + 1;
        int k = 0;
        if (action) g_shots[g_nshots].path[k++] = '!';
        while (*s && *s != ',' && k < MAX_PATH - 1) g_shots[g_nshots].path[k++] = *s++;
        g_shots[g_nshots++].path[k] = 0;
        if (*s == ',') s++;
    }
}

static void civnet_start(struct CPU *cpu)
{
    (void)cpu;
    parse_due(getenv("CIVNET_SHOT"), 0);
    parse_due(getenv("CIVNET_KEYS"), 1);
    qsort(g_shots, g_nshots, sizeof g_shots[0], by_time);
    if (g_nshots) CloseHandle(CreateThread(NULL, 0, shot_thread, NULL, 0, NULL));
}

const RecompGame recomp_game = {
    .start      = civnet_start,
    .title      = "CivNet",
    .guest_dir  = "C:\\CIVNET",
    .ini_fallbacks = { "CIVNET.INI", NULL },
    /* CIVENG0.DLL maps 4-character resource tags ("STRN") to numeric types
     * with [00 00][0C 00][type][id][tag] records: type and id come before
     * the tag (2), the other way round from PF Magic's. */
    .resource_tags = 2,
    /* Real child and pop-up windows that it moves and sizes (the intro movie,
       its zooming dialogs): each keeps its own geometry. */
    .real_windows = 1,
    /* Its framework unlinks a window from its parent in WM_DESTROY. */
    .destroy_message = 1,
    /* Its WinG surfaces show through the palette it realises and animates. */
    .wing_screen_palette = 1,
    /* ... with the system's 20 static colours, which its dialogs use. */
    .static_colors = 1,
    .proc_address = proc_address,
};
