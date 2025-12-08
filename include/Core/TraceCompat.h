#pragma once

#include <cstdarg>
#include <cstdio>

// Log levels (matching raylib convention)
#define LOG_ALL 0
#define LOG_TRACE 1
#define LOG_DEBUG 2
#define LOG_INFO 3
#define LOG_WARNING 4
#define LOG_ERROR 5
#define LOG_FATAL 6
#define LOG_NONE 7

inline void ProjectTraceLog(int /*level*/, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::vfprintf(stderr, fmt, args);
  std::fputc('\n', stderr);
  va_end(args);
}

#ifndef TRACELOG
#define TRACELOG ProjectTraceLog
#endif
