#ifndef LUASHELL_H
#define LUASHELL_H

#pragma once

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

    static inline void set_color(int color) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
            SetConsoleTextAttribute(hConsole, (WORD)color);
        }
    }

    #define TERM_GREEN  set_color(10)
    #define TERM_RED    set_color(12)
    #define TERM_BLUE   set_color(9)
    #define TERM_YELLOW set_color(14)
    #define TERM_CYAN   set_color(11)
    #define TERM_RESET  set_color(7)

#else
    #include <stdio.h>

    #define TERM_GREEN  printf("\033[1;32m")
    #define TERM_RED    printf("\033[1;31m")
    #define TERM_BLUE   printf("\033[1;34m")
    #define TERM_YELLOW printf("\033[1;33m")
    #define TERM_CYAN   printf("\033[1;36m")
    #define TERM_RESET  printf("\033[0m")
#endif

#endif