#ifndef LOG_H
#define LOG_H

#define LOG_LEVEL_ERR 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_PLAID 99

void logmsg(int level, const char* fmt, ...) __attribute__((format (printf, 2,3)));

#define ERR(fmt, ...) logmsg(LOG_LEVEL_ERR, fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) logmsg(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) logmsg(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define DBG(fmt, ...) logmsg(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)

#endif

