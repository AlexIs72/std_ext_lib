#ifndef __PID_FILE_HPP__
#define __PID_FILE_HPP__

#include <string>

class pid_file
{
    public:
        pid_file ();
        virtual ~pid_file ();

        inline int create(const std::string &app_name)
        {
            return create(app_name.c_str());
        }
        int create(const char *app_name);
        int remove();

        inline const std::string & get_pid_file() const
        {
            return m_pid_file;
        }


    private:
        int m_pid;
        std::string     m_pid_file;    
            
        /* private data */
};

#endif /* end of include guard PID_FILE_HPP */

