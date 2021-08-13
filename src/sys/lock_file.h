#ifndef __LOCK_FILE_HPP__
#define __LOCK_FILE_HPP__

#include <string>


class lock_file
{
    public:
        lock_file ();
        virtual ~lock_file ();
        
        inline int try_lock(const std::string &app_name)
        {
            return try_lock(app_name.c_str());
        }
        int try_lock(const char *app_name);
        int unlock();

        inline const std::string & get_lock_file() const
        {
            return m_lock_file;
        }

    private:

        int m_fd;
        std::string m_lock_file;
        /* private data */
};

#endif /* end of include guard LOCK_FILE_HPP */

