/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 16:03:46
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-08 14:40:59
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/log.h
 */
#pragma once

#include <glibmm.h>

#include "lib/helper.h"

namespace Kiran
{
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

class ILogger
{
public:
    virtual void write_log(const char *buff, uint32_t len) = 0;
};

class Log
{
public:
    Log();
    Log(const Log &) = delete;
    virtual ~Log();

    static Log *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit();

    inline GLogLevelFlags get_level() { return this->log_level_; }

    void set_log_level(GLogLevelFlags log_level) { this->log_level_ = log_level; }

    void set_logger(ILogger *logger) { logger_ = logger; }

    void try_append(GLogLevelFlags log_level, const char *format, ...);

private:
    void init();

    static void log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data);

private:
    static Log *instance_;

    GLogLevelFlags log_level_;

    ILogger *logger_;

    static const int kMessageSize = 10240;
    char message_[kMessageSize];
};

#define LOG_FATAL(fmt, ...)                                                                          \
    do                                                                                               \
    {                                                                                                \
        Kiran::Log::get_instance()->try_append(G_LOG_FLAG_FATAL, "%s:%d-%s() " fmt,                  \
                                               __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
    } while (0)

#define LOG_ERROR(fmt, ...)                                                                          \
    do                                                                                               \
    {                                                                                                \
        Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_ERROR, "%s:%d-%s() " fmt,                 \
                                               __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
    } while (0)

#define LOG_CRITICAL(fmt, ...)                                                                       \
    do                                                                                               \
    {                                                                                                \
        Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_CRITICAL, "%s:%d-%s() " fmt,              \
                                               __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
    } while (0)

#define LOG_WARNING(fmt, ...)                                                                        \
    do                                                                                               \
    {                                                                                                \
        Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_WARNING, "%s:%d-%s() " fmt,               \
                                               __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
    } while (0)

#define LOG_MESSAGE(fmt, ...)                                                                        \
    do                                                                                               \
    {                                                                                                \
        Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_MESSAGE, "%s:%d-%s() " fmt,               \
                                               __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
    } while (0)

#define LOG_INFO(fmt, ...)                                                                           \
    do                                                                                               \
    {                                                                                                \
        Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_INFO, "%s:%d-%s() " fmt,                  \
                                               __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
    } while (0)

#define LOG_DEBUG(fmt, ...)                                                                          \
    do                                                                                               \
    {                                                                                                \
        Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_DEBUG, "%s:%d-%s() " fmt,                 \
                                               __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
    } while (0)

#define SETTINGS_PROFILE_START(fmt, ...)                                                             \
    do                                                                                               \
    {                                                                                                \
        Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_DEBUG, "%s:%d-%s() START " fmt,           \
                                               __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
    } while (0)

#define SETTINGS_PROFILE_END(fmt, ...)                                                               \
    do                                                                                               \
    {                                                                                                \
        Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_DEBUG, "%s:%d-%s() END " fmt,             \
                                               __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
    } while (0)

#define SETTINGS_PROFILE(fmt, ...)                                                                  \
    Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_DEBUG, "%s:%d-%s() START " fmt,              \
                                           __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__);    \
    SCOPE_EXIT({ Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_DEBUG, "%s:%d-%s() END   " fmt, \
                                                        __FILENAME__, __LINE__, _arg_function.c_str(), ##__VA_ARGS__); });

}  // namespace Kiran