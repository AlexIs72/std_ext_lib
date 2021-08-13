#include <iostream>
#include <sstream>
#include <chrono>
#include <vector>
#include <iomanip>

#include <stdarg.h>

#include "logger.h"

//#include "../types/typedefs.h"

#define CONS_BLACK          30
#define CONS_RED            31
#define CONS_GREEN          32
#define CONS_YELLOW         33
#define CONS_BLUE           34
#define CONS_PURPLE         35
#define CONS_CYAN           36
#define CONS_WHITE          37

//#define TO_STR(a)   #a
//#define CONCAT_COLOR(a,b,c) a ## b ## c
#define COLOURF(cletter) "\033[" #cletter "m"
//CONCAT_COLOR("\033[", #cletter, "m")
//#define CHOOSE_COLOR(is_left, left, right) COLOURF( ( is_left ? left : right ) )
#define CRESET() "\033[0m"
//#define FORMAT_STR(str) QString(str).toStdString()


const int level_color[LL_LEVELS_COUNT] = 
{
    CONS_WHITE, // LL_RAW:
    CONS_GREEN, // LL_INFO:
    CONS_YELLOW, // LL_WARNING:
    CONS_RED, // LL_ERROR:
    CONS_PURPLE// LL_DEBUG:
};

const char *level_str[LL_LEVELS_COUNT] = 
{
    "raw", // LL_RAW:
    "info", // LL_INFO:
    "warning", // LL_WARNING:
    "error", // LL_ERROR:
    "debug"// LL_DEBUG:
};

/*
https://github.com/fmtlib/fmt
*/

/*logger::logger()
{
    
}

logger::~logger()
{
}
*/

void logger::write(log_level level, const char *fmt, ...)
{
//    int ret;
//    UNUSED(level);
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
//    std::time_t t = std::time(nullptr);
//    char time_buf[100];
//    std::strftime(time_buf, sizeof time_buf, "%D %T", std::gmtime(&t));
    va_list args1;
    va_start(args1, fmt);
    va_list args2;
    va_copy(args2, args1);
    std::vector<char> buf(1+std::vsnprintf(nullptr, 0, fmt, args1));
    va_end(args1);
    std::vsnprintf(buf.data(), buf.size(), fmt, args2);
    va_end(args2);
    std::cout << 
//        COLOURF(level_color[level]) <<
        "\033[" << level_color[level] << "m" << 
        std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") <<
        " [" << level_str[level] << "] " << 
        buf.data() <<
        CRESET() << 
        std::endl;

#if 0

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
    //return ss.str();


    /* Declare a va_list type variable */
    va_list myargs;

    /* Initialise the va_list variable with the ... after fmt */

    va_start(myargs, fmt);

    /* Forward the '...' to vprintf */
    /*ret = */vprintf(fmt, myargs);

    /* Clean up the va_list */
    va_end(myargs);
#endif
//    return ret;
//    std::ostringstream oss;
//    write(oss, args...);
//    std::cout << oss.str();    
}