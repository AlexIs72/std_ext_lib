#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

typedef enum {
    LL_RAW,
    LL_INFO,
    LL_WARNING,
    LL_ERROR,
    LL_DEBUG,
    // ..
    LL_LEVELS_COUNT //must be last
} log_level;


class logger
{
    public:
//        logger ();
//        virtual ~logger ();

        static void write(log_level level, const char *fmt, ...);
        template<typename... Args> static void raw(const char *fmt, Args... args) {
            write(LL_RAW, fmt, args...);
        }
        template<typename... Args> static void info(const char *fmt, Args... args) {
            write(LL_INFO, fmt, args...);
        }
        template<typename... Args> static void warning(const char *fmt, Args... args) {
            write(LL_WARNING, fmt, args...);
        }
        template<typename... Args> static void error(const char *fmt, Args... args) {
            write(LL_ERROR, fmt, args...);
        }
        template<typename... Args> static void debug(const char *fmt, Args... args) {
            write(LL_DEBUG, fmt, args...);
        }
    private:
};

#endif /* end of include guard LOGGER_HPP */

