#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>

#include "lock_file.h"

static inline int error_handler(int *fd, int ret_code)
{
    close(*fd);
    *fd = -1;
    return ret_code;
}

lock_file::lock_file() :
    m_fd(0)
{
    
}

lock_file::~lock_file()
{
    if(m_fd > 0)
    {
        unlock();
    }
}

int lock_file::try_lock(const char *app_name)
{
    if(m_fd > 0)
    {
        return -1;
    }

    char buf[10] = {0};
    int pid = getpid();
    int len;

    m_lock_file = "/tmp/"; // "/var/lock/";
    m_lock_file += app_name;
    m_lock_file += ".lock";

    len = snprintf(buf, sizeof(buf), "%d", pid);

    mode_t m = umask( 0 );
    m_fd = open( m_lock_file.c_str(), O_RDWR|O_CREAT, 0666 );
    umask( m );
    if( m_fd >= 0 && flock( m_fd, LOCK_EX | LOCK_NB ) < 0 )
    {
//        close( m_fd );
//        m_fd = -1;
//        return m_fd;
        return error_handler(&m_fd, -1);
    }

    if(write(m_fd, buf, len) < len)
    {
        return error_handler(&m_fd, -1);
    }

    return 0;
}
                                                                                                                 
int lock_file::unlock()
{
    if(m_fd <= 0)
    {
        return -1;
    }    

    remove( m_lock_file.c_str() );
    close( m_fd );
    m_fd = 0;    

    return 0;
}


/*
void testlock(void) {
  # pragma omp parallel num_threads(160)
  {    
    int fd = -1; char ln[] = "testlock.lock";
    while (fd == -1) fd = tryGetLock(ln);

    cout << omp_get_thread_num() << ": got the lock!";
    cout << omp_get_thread_num() << ": removing the lock";

    releaseLock(fd,ln);
  }
}

int tryGetLock( char const *lockName)
{
    mode_t m = umask( 0 );
    int fd = open( lockName, O_RDWR|O_CREAT, 0666 );
    umask( m );
    printf("Opened the file. Press enter to continue...");
    fgetc(stdin);
    printf("Continuing by acquiring the lock.\n");
    if( fd >= 0 && flock( fd, LOCK_EX | LOCK_NB ) < 0 )
    {
        close( fd );
        fd = -1;
    }
    return fd;
}

static const char *lockfile = "/tmp/mylock.lock";

int main(int argc, char *argv[0])
{
    int lock = tryGetLock(lockfile);
    if (lock == -1) {
        printf("Getting lock failed\n");
        return 1;
    }

    printf("Acquired the lock. Press enter to release the lock...");
    fgetc(stdin);

    printf("Releasing...");
    releaseLock(lock, lockfile);
    printf("Done!\n");
    return 0;

}


int tryGetLock( char const *lockName )
{
}
and:

void releaseLock( int fd, char const *lockName )
{
    if( fd < 0 )
        return;
    remove( lockName );
    close( fd );
}


*/