#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static int l_sleep(lua_State *L) {
    lua_Integer ms = luaL_checkinteger(L, 1);
    if (ms < 0) ms = 0;
    const lua_Integer MAX_MS = 60 * 1000;
    if (ms > MAX_MS) ms = MAX_MS;
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

static int l_beep(lua_State *L) {
    lua_Integer freq = luaL_checkinteger(L, 1);
    lua_Integer dur  = luaL_checkinteger(L, 2);

    if (freq < 37) freq = 37;
    if (freq > 32767) freq = 32767;
    if (dur < 0) dur = 0;
    if (dur > 60000) dur = 60000;

#ifdef _WIN32
    if (!Beep((DWORD)freq, (DWORD)dur)) {
        fprintf(stderr, "\033[1;31mWARNING:\033[0m Beep failed (freq=%lld dur=%lld)\n",
                (long long)freq, (long long)dur);
    }
#else
    (void)freq; (void)dur;
    fprintf(stderr, "\033[1;33mINFO:\033[0m cbeep not implemented on this platform\n");
#endif
    return 0;
}

static void Execute(lua_State *L, const char *Code) {
    if (!L || !Code) return;
    int status = luaL_dostring(L, Code);
    if (status != LUA_OK) {
        const char *msg = lua_tostring(L, -1);
        fprintf(stderr, "\033[1;31mLUA ERROR:\033[0m %s\n", msg ? msg : "unknown");
        lua_pop(L, 1);
    }
}

static int RunFile(lua_State *L, const char *filename) {
    if (!L || !filename) return 1;

    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "\033[1;31mFATAL ERROR:\033[0m cannot open file '%s'\n", filename);
        return 1;
    }
    fclose(f);

    int status = luaL_dofile(L, filename);
    if (status != LUA_OK) {
        const char *msg = lua_tostring(L, -1);
        fprintf(stderr, "\033[1;31mFATAL ERROR:\033[0m %s\n", msg ? msg : "unknown");
        lua_pop(L, 1);
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
#endif

    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        printf("\033[1;32mLuaIC - Lua In C\033[0m\n");
        printf("2026 - 2026\n");
        printf("By @LostArduino (LostMind)\n");
        printf("Usage: luaic [file.lua]\n");
        return 0;
    }

    lua_State *L = luaL_newstate();
    if (!L) {
        fprintf(stderr, "\033[1;31mFATAL ERROR:\033[0m cannot create Lua state: not enough memory\n");
        return 1;
    }

    luaL_openlibs(L);

    lua_register(L, "csleep", l_sleep);
    lua_register(L, "cbeep",  l_beep);

    int exit_code = 0;
    if (argc > 1) {
        if (RunFile(L, argv[1]) != 0) {
            exit_code = 1;
        }
    } else {
        printf("\033[1;32mLuaIC 1.2\033[0m | \033[1;34mShell\033[0m | \033[1;33mLua 5.5\033[0m\n");
        printf("Built-in: \033[1;36mcsleep(ms)\033[0m, \033[1;36mcbeep(f, d)\033[0m\n");
        printf("Usage: luaic [file.lua] or type 'exit()'\n");

        while (1) {
            printf("\033[1;33m> \033[0m");
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
    return exit_code;
}