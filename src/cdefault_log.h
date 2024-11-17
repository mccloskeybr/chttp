#ifndef CDEFAULT_LOG_H
#define CDEFAULT_LOG_H

#include <stdarg.h>
#include <stdio.h>
#include <inttypes.h>

void Log(FILE* fd, char* level, char* filename, int32_t loc, char* fmt, ...);
void LogV(FILE* fd, char* level, char* filename, int32_t loc, char* fmt, va_list args);

#define CDEFAULT_LOG_FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define CDEFAULT_LOG(fd, level, fmt, ...) \
  Log(fd, level, CDEFAULT_LOG_FILENAME, __LINE__, fmt, __VA_ARGS__)
#define LOG_INFO(fmt, ...) CDEFAULT_LOG(stdout, "INFO", fmt, __VA_ARGS__)
#define LOG_WARN(fmt, ...) CDEFAULT_LOG(stdout, "WARN", fmt, __VA_ARGS__)
#define LOG_ERROR(fmt, ...) CDEFAULT_LOG(stderr, "ERROR", fmt, __VA_ARGS__)
#define LOG_FATAL(fmt, ...) { CDEFAULT_LOG(stderr, "FATAL", fmt, __VA_ARGS__); exit(1); }

#ifdef DEBUG
#define LOG_DEBUG(fmt, ...) Log(stdout, "DEBUG", fmt, __VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)
#endif

#ifdef CDEFAULT_LOG_IMPL

void Log(FILE* fd, char* level, char* filename, int32_t loc, char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  LogV(fd, level, filename, loc, fmt, args);
  va_end(fmt);
}

void LogV(FILE* fd, char* level, char* filename, int32_t loc, char* fmt, va_list args) {
  fprintf(fd, "[%s|%s:%d]: ", level, filename, loc);
  vfprintf(fd, fmt, args);
  fprintf(fd, "\n");
}

#endif
#endif
