#ifndef DEFINES_H
#define DEFINES_H

#if defined(_WIN32) || defined(_WIN64)
#define windows
#include <Windows.h>
#endif

#if defined(__APPLE__)
#define apple
#endif

#ifdef _WIN32
#define sleep(x) Sleep((x))
#else
#define sleep(x) usleep((x)*1000)
#endif

#endif // DEFINES_H
