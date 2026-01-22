# LuaShell / LuaInC!
Lua Interpreter Writed on C!


**Compile:** `MakeFile`, **requirements:** `MinGW` or `GCC` / `TCC`

**GCC Compile:** `make gcc`, **MINGW-X64 Compile:** `make mingw`, **TCC Compile:** `make tcc`

**MINGW From Scoop:** `gcc LuaShell.c LuaBins/liblua55.a -o luaic.exe -I./LuaIncludes -static -lm`

### C Functions:
* `csleep(ms)` — System Sleep
* `cbeep(freq, dur)` — System Beep!
* `ccls()` — Clears Console


