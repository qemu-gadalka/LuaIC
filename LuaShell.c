#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "LuaIncludes/lua.h"
#include "LuaIncludes/lualib.h"
#include "LuaIncludes/lauxlib.h"

char* read_input() {
    size_t capacity = 128, length = 0;
    char *buffer = malloc(capacity);
    if (!buffer) return NULL;
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (length + 1 >= capacity) {
            capacity *= 2;
            char *new_ptr = realloc(buffer, capacity);
            if (!new_ptr) { free(buffer); return NULL; }
            buffer = new_ptr;
        }
        buffer[length++] = (char)ch;
    }
    if (length == 0 && ch == EOF) { free(buffer); return NULL; }
    buffer[length] = '\0';
    return buffer;
}

int l_sleep(lua_State *L) {
    int ms = (int)luaL_checknumber(L, 1); 
    Sleep(ms);
    return 0;
}

int l_beep(lua_State *L) {
    int freq = (int)luaL_checknumber(L, 1);
    int dur = (int)luaL_checknumber(L, 2);
    Beep(freq, dur);
    return 0;
}

void Execute(lua_State *L, const char *Code) {
    if (luaL_dostring(L, Code) != 0) {
        fprintf(stderr, "\033[1;31mLUA ERROR:\033[0m %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

void RunFile(lua_State *L, const char *filename) {
    if (luaL_dofile(L, filename) != 0) {
        fprintf(stderr, "\033[1;31mFATAL ERROR:\033[0m %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        printf("\033[1;32mLuaIC - Lua In C\033[0m\n");
        printf("2026 - 2026\n");
        printf("By @LostArduino (LostMind)\n");
        printf("Usage: luaic [file.lua]\n");
        return 0;
    }

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    lua_register(L, "csleep", l_sleep);
    lua_register(L, "cbeep", l_beep);

    if (argc > 1) {
        RunFile(L, argv[1]);
    } 
    else {
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
    return 0;
}