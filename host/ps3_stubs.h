// Stand-ins for the PS3-only calls main.cpp makes, so it builds on the Mac (make mac); logs go to stdout.
#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum { ERROR, INFO, DEBUG };

inline void netInitialize() {}
inline void debugNetInit(const char*, int, int) {}
inline void debugNetFinish() {}

inline void debugNetPrintf(int level, const char* format, ...) {
    printf("[%s] ", level == ERROR ? "ERROR" : level == INFO ? "INFO" : "DEBUG");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}

// No watchdog on the Mac: the thread never starts, so the RSX stand-ins below only have to compile.
typedef int sys_ppu_thread_t;
inline int sysThreadCreate(sys_ppu_thread_t*, void (*)(void*), void*, int, size_t, int, const char*) { return 0; }
inline void sysUsleep(unsigned microseconds) { usleep(microseconds); }
inline void sysProcessExit(int code) { exit(code); }

struct gcmControlRegister { unsigned put, get; };
inline gcmControlRegister* gcmGetControlRegister() { static gcmControlRegister rsx; return &rsx; }
inline unsigned gcmGetFlipStatus() { return 0; }

struct MallInfo { int uordblks; };
inline MallInfo mallinfo() { return MallInfo{0}; }
