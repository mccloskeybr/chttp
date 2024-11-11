#ifndef MACROS_H
#define MACROS_H

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define LOG(level, fd, fmt, ...)                               \
  {                                                            \
    fprintf(fd, "[%s|%s:%d] ", level, __FILENAME__, __LINE__); \
    fprintf(fd, fmt, __VA_ARGS__);                             \
    fprintf(fd, "\n");                                         \
  }
#define LOG_INFO(fmt, ...) LOG("INFO", stdout, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG("ERROR", stderr, fmt, ##__VA_ARGS__)
#define PANIC(fmt, ...) { LOG_ERROR(fmt, ##__VA_ARGS__); exit(1); }

#endif
