#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sstream>

#include "pid_file.h"

pid_file::pid_file() :
    m_pid(-1)
{
    
}

pid_file::~pid_file()
{
    if(m_pid >= 0)
    {
        remove();
    }
}

int pid_file::create(const char *app_name)
{
    if(m_pid >= 0)
    {
        return -1;
    }

    FILE* f;
    struct stat sb;
    std::ostringstream sstream;
    
    sstream << "/run/user/" << getuid() << "/" << app_name << ".pid";
    m_pid_file = sstream.str();
//  std::string copyOfStr = stringStream.str();

//    char buf[10] = {0};

//    m_pid_file = "/run/user/";
//    m_pid_file += app_name;
//    m_pid_file += ".pid";

    if (stat(m_pid_file.c_str(), &sb) == 0) {
        return -1;
    }

    m_pid = getpid();
    f = fopen(m_pid_file.c_str(), "w+");
    if (f)
    {
        fprintf(f, "%d", m_pid);
        fclose(f);
    }

    return 0;
}

int pid_file::remove()
{
    if(m_pid < 0)
    {
        return -1;
    }

    ::remove(m_pid_file.c_str());

    m_pid_file = -1;

    return 0;
}