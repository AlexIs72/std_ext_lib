#ifndef __APP_HANDLER_HPP__
#define __APP_HANDLER_HPP__

#include <signal.h>

#include <string>
#include <vector>
#include <csignal>

#include <sys/lock_file.h>
#include <sys/pid_file.h>

#include "core_application.h"

#ifndef sighandler_t
    typedef void (*sighandler_t)(int);
#endif

class app_handler
{
    public:
        app_handler ();
        virtual ~app_handler ();
        
        int parse_args(int argc, char *argv[]);
        void set_app_version(const char *version, const char *revision);
        int run(core_application &app);

        inline const char *app_name() const
        {
            return m_app_name.c_str();
        }

        inline const std::vector<std::string> &app_args() const
        {
            return m_app_args;
        }


    private:
        void print_version() const;        
//        static void exit(int signal);
        
        std::vector<std::string> m_app_args;
        std::string              m_app_name;    
        std::string              m_app_version;
        std::string              m_app_revision;
        std::string              m_uniq_app_id;
        lock_file                m_app_lock;
        pid_file                 m_app_pid;
        sighandler_t             m_old_sigint;
        sighandler_t             m_old_sigtem;
        sighandler_t             m_old_sigabrt;
        sighandler_t             m_old_sigquit;

//        static int               m_exit_flag;
//        static core_application  *m_app;
};

#endif /* end of include guard APP_HANDLER_HPP */

