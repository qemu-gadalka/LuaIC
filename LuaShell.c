#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "LuaIncludes/lua.h"
#include "LuaIncludes/lauxlib.h"
#include "LuaIncludes/lualib.h"
#include "Includes/LuaShell.h"

static char *read_input(void) {
    size_t capacity = 128;
    size_t length = 0;
    char *buffer = malloc(capacity);
    if (!buffer) return NULL;

    int ch;
    while ((ch = getchar()) != EOF && ch != '\n') {
        if (length + 1 >= capacity) {
            size_t new_capacity = capacity * 2;
            char *new_ptr = realloc(buffer, new_capacity);
            if (!new_ptr) { free(buffer); return NULL; }
            buffer = new_ptr;
            capacity = new_capacity;
        }
        buffer[length++] = (char)ch;
    }
    if (length == 0 && ch == EOF) { free(buffer); return NULL; }
    buffer[length] = '\0';
    return buffer;
}

static int l_settitle(lua_State *L) {
    const char *title = luaL_checkstring(L, 1);
#ifdef _WIN32
    SetConsoleTitleA(title);
#else
    printf("\033]0;%s\007", title);
#endif
    return 0;
}

static int l_setcursorvisible(lua_State *L) {
    int visible = lua_toboolean(L, 1);
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = visible;
    SetConsoleCursorInfo(hOut, &cursorInfo);
#else
    printf(visible ? "\033[?25h" : "\033[?25l");
#endif
    return 0;
}

static int l_gotoxy(lua_State *L) {
    int x = (int)luaL_checkinteger(L, 1);
    int y = (int)luaL_checkinteger(L, 2);
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hOut, coord);
#else
    printf("\033[%d;%dH", y + 1, x + 1);
#endif
    return 0;
}

static int l_sleep(lua_State *L) {
    lua_Integer ms = luaL_checkinteger(L, 1);
    if (ms < 0) ms = 0;
#ifdef _WIN32
    Sleep((DWORD)ms);
#else
    struct timespec ts;
    ts.tv_sec = (time_t)(ms / 1000);
    ts.tv_nsec = (long)((ms % 1000) * 1000000);
    nanosleep(&ts, NULL);
#endif
    return 0;
}

static int l_cls(lua_State *L) {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[H\033[J");
#endif
    return 0;
}

static int l_beep(lua_State *L) {
    lua_Integer freq = luaL_checkinteger(L, 1);
    lua_Integer dur  = luaL_checkinteger(L, 2);
    if (freq < 37) freq = 37;
    if (freq > 32767) freq = 32767;
#ifdef _WIN32
    Beep((DWORD)freq, (DWORD)dur);
#else
    TERM_YELLOW; printf("INFO: "); TERM_RESET;
    printf("cbeep not implemented on this platform\n");
#endif
    return 0;
}

static void Execute(lua_State *L, const char *Code) {
    if (!L || !Code) return;
    if (luaL_dostring(L, Code) != LUA_OK) {
        const char *msg = lua_tostring(L, -1);
        TERM_RED; printf("LUA ERROR: "); TERM_RESET;
        printf("%s\n", msg ? msg : "unknown");
        lua_pop(L, 1);
    }
}

static int RunFile(lua_State *L, const char *filename) {
    if (!L || !filename) return 1;
    if (luaL_dofile(L, filename) != LUA_OK) {
        const char *msg = lua_tostring(L, -1);
        TERM_RED; printf("FATAL ERROR: "); TERM_RESET;
        printf("%s\n", msg ? msg : "unknown");
        lua_pop(L, 1);
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &dwMode)) {
        SetConsoleMode(hOut, dwMode | 0x0004);
    }
#endif

    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        TERM_GREEN; printf("LuaIC - Lua In C\n"); TERM_RESET;
        printf("2026 | By @LostArduino\n");
        printf("Usage: luaic [file.lua]\n");
        return 0;
    }

    lua_State *L = luaL_newstate();
    if (!L) return 1;
    luaL_openlibs(L);

    lua_register(L, "ccls", l_cls);
    lua_register(L, "csleep", l_sleep);
    lua_register(L, "cbeep",  l_beep);

    lua_register(L, "csettitle", l_settitle);
    lua_register(L, "cgotoxy", l_gotoxy);
    
    lua_pushboolean(L, 0); lua_register(L, "chidecursor", l_setcursorvisible);
    lua_pushboolean(L, 1); lua_register(L, "cshowcursor", l_setcursorvisible);
    if (argc > 1) {
        if (RunFile(L, argv[1]) != 0) return 1;
    } else {
        TERM_GREEN; printf("LuaIC 1.3 "); TERM_RESET;
        printf("| ");
        TERM_BLUE; printf("Shell "); TERM_RESET;
        printf("| ");
        TERM_YELLOW; printf("Lua 5.5\n"); TERM_RESET;

        while (1) {
            TERM_YELLOW; printf("> "); TERM_RESET;
            char *input = read_input();
            if (!input) break;
            
            if (strcmp(input, "exit()") == 0 || strcmp(input, "quit()") == 0) {
                free(input);
                break;
            }
            Execute(L, input);
            free(input);
        }
    }

    lua_close(L);
    return 0;
}
