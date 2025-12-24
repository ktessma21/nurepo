#pragma once
/*
  Single-file logging macros for C.

  Usage:
    #define LOG_LEVEL LOG_DEBUG   // optional, before including log.h
    #define LOG_ENABLE_DEBUG      // optional, to enable DEBUG()
    #include "log.h"

    INFO("hi %d", 123);
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ---- Levels (lower = more verbose) ---- */
enum {
    LOG_DEBUG = 0,
    LOG_INFO  = 1,
    LOG_WARN  = 2,
    LOG_ERROR = 3,
    LOG_NONE  = 99
};

/* Compile-time threshold (default INFO) */
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_INFO
#endif

/* Timestamp helper (HH:MM:SS) */
static inline const char *log__timestamp(void)
{
    static char buf[16];
    time_t t = time(NULL);
    struct tm tmv;
#if defined(_POSIX_VERSION)
    localtime_r(&t, &tmv);
#else
    tmv = *localtime(&t);
#endif
    strftime(buf, sizeof(buf), "%H:%M:%S", &tmv);
    return buf;
}

static inline const char *log__level_name(int lvl)
{
    switch (lvl) {
    case LOG_DEBUG: return "DEBUG";
    case LOG_INFO:  return "INFO";
    case LOG_WARN:  return "WARN";
    case LOG_ERROR: return "ERROR";
    default:        return "?";
    }
}

static inline const char *log__level_name_safe(int lvl)
{
    const char *s = log__level_name(lvl);
    return s ? s : "?";
}

/* Core macro */
#define LOG(lvl, fmt, ...)                                                     \
    do {                                                                       \
        if ((lvl) >= LOG_LEVEL) {                                              \
            fprintf(stderr, "%s %-5s %s:%d: " fmt "\n",                        \
                    log__timestamp(),                                          \
                    log__level_name_safe(lvl),                                 \
                    __FILE__,                                                  \
                    __LINE__,                                                  \
                    ##__VA_ARGS__);                                            \
        }                                                                      \
    } while (0)


/* Convenience macros */
#define INFO(fmt, ...)  LOG(LOG_INFO,  fmt, ##__VA_ARGS__)
#define WARN(fmt, ...)  LOG(LOG_WARN,  fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) LOG(LOG_ERROR, fmt, ##__VA_ARGS__)

/* DEBUG is optional (off by default unless LOG_ENABLE_DEBUG is defined) */
#ifdef LOG_ENABLE_DEBUG
#define DEBUG(fmt, ...) LOG(LOG_DEBUG, fmt, ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...) do { } while (0)
#endif

/* Progress (no newline) */
#define PROGRESS(fmt, ...)                       \
    do {                                         \
        fprintf(stderr, "\r" fmt, ##__VA_ARGS__);\
        fflush(stderr);                          \
    } while (0)

/* Fatal */
#define DIE(fmt, ...)                            \
    do {                                         \
        ERROR(fmt, ##__VA_ARGS__);               \
        exit(1);                                 \
    } while (0)
