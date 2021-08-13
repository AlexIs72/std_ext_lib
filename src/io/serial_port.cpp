#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>

#include <cassert>
#include <limits>
#include <iostream>

//#include <types/typedefs.h>

#include "serial_port.h"

#define DEFAULT_INPUT_BUF_SIZE  512
//#define INVALID_PORT_HANDLE     0

/**
 * @brief Character used to signal that I/O can start while using
 *        software flow control with the serial port.
 */
constexpr char CTRL_Q = 0x11 ;

/**
 * @brief Character used to signal that I/O should stop while using
 *        software flow control with the serial port.
 */
constexpr char CTRL_S = 0x13 ;

serial_port::serial_port() :
    m_ph(INVALID_PORT_HANDLE),
    m_baud_rate(baud_rate::BAUD_DEFAULT),
    m_char_size(char_size::CHAR_SIZE_DEFAULT),
    m_parity(parity::PARITY_DEFAULT),   
    m_stop_bits(stop_bits::STOP_BITS_DEFAULT),
    m_flow_control(flow_control::FLOW_CONTROL_DEFAULT),
    m_exit_flag(false),
    m_on_read_complete_cb(nullptr),
    m_on_error_cb(nullptr)
{
}

serial_port::~serial_port()
{
    close();
}

int serial_port::open(const char *name, open_mode om)
{
    int flags = O_NOCTTY | O_NONBLOCK;
    struct stat sb;

    if (stat(name, &sb) == -1) 
    {
        return -1;
    }

    switch(om)
    {
        case open_mode::OPEN_MODE_RD:
            flags |= O_RDONLY;
            break;
        case open_mode::OPEN_MODE_RW:
            flags |= O_RDWR;
            break;
        default:
            return -1;
    }



    m_ph = ::open(name, flags/*O_RDWR | O_NOCTTY | O_NONBLOCK*/);   
  
    if(m_ph <=0)
    {
        perror("open: ");
        m_ph = INVALID_PORT_HANDLE;
        return -1;
    }

    if(set_port_settings() != 0)
    {
        perror("set_port_settings:  ");
        return -1;
    }

    m_exit_flag = false;

    return 0;
}

int serial_port::set_port_settings()
{
    if(m_ph == INVALID_PORT_HANDLE)
    {
        return -1;
    }

    struct termios port_settings = {};
    memset(&port_settings, 0, sizeof(struct termios));

    if(tcgetattr(m_ph, &port_settings) != 0) 
    {
//        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return -1;
    }

    if (cfsetspeed(&port_settings, static_cast<speed_t>(m_baud_rate)) != 0)
    {
        return -1;
            // If applying the baud rate settings fail, throw an exception.
//        throw std::runtime_error(ERR_MSG_INVALID_BAUD_RATE) ;
    }

    if (m_char_size == char_size::CHAR_SIZE_8)
    {
        // NOLINTNEXTLINE (hicpp-signed-bitwise)
        port_settings.c_iflag &= ~ISTRIP ;  // Clear the ISTRIP flag.
    }
    else
    {
        port_settings.c_iflag |= ISTRIP ;   // Set the ISTRIP flag.
    }

    // Set the character size.
    // NOLINTNEXTLINE (hicpp-signed-bitwise)
    port_settings.c_cflag &= ~CSIZE ;                               // Clear all CSIZE bits.
    port_settings.c_cflag |= static_cast<tcflag_t>(m_char_size) ; // Set the character size.

    switch(m_flow_control)
    {
        case flow_control::FLOW_CONTROL_HARDWARE:
            port_settings.c_iflag &= ~ (IXON|IXOFF) ;   // NOLINT (hicpp-signed-bitwise)
            port_settings.c_cflag |= CRTSCTS ;
            port_settings.c_cc[VSTART] = _POSIX_VDISABLE ;
            port_settings.c_cc[VSTOP] = _POSIX_VDISABLE ;
            break ;
        case flow_control::FLOW_CONTROL_SOFTWARE:
            port_settings.c_iflag |= IXON|IXOFF ;        // NOLINT(hicpp-signed-bitwise)
            port_settings.c_cflag &= ~CRTSCTS ;
            port_settings.c_cc[VSTART] = CTRL_Q ;        // 0x11 (021) ^q
            port_settings.c_cc[VSTOP]  = CTRL_S ;        // 0x13 (023) ^s
            break ;
        case flow_control::FLOW_CONTROL_NONE:
            port_settings.c_iflag &= ~(IXON|IXOFF) ;    // NOLINT(hicpp-signed-bitwise)
            port_settings.c_cflag &= ~CRTSCTS ;
            break ;
        default:
            return -1;
    //        throw std::invalid_argument(ERR_MSG_INVALID_FLOW_CONTROL) ;
            // break ; break not needed after a throw
    }

    switch(m_parity)
    {
        case parity::PARITY_EVEN:
            port_settings.c_cflag |= PARENB ;
            port_settings.c_cflag &= ~PARODD ;  // NOLINT (hicpp-signed-bitwise)
            port_settings.c_iflag |= INPCK ;
            break ;
        case parity::PARITY_ODD:
            port_settings.c_cflag |= PARENB ;
            port_settings.c_cflag |= PARODD ;
            port_settings.c_iflag |= INPCK ;
            break ;
        case parity::PARITY_NONE:
            port_settings.c_cflag &= ~PARENB ;  // NOLINT (hicpp-signed-bitwise)
            port_settings.c_iflag |= IGNPAR ;
            break ;
        default:
            return -1;
//            throw std::invalid_argument(ERR_MSG_INVALID_PARITY) ;
            // break ; break not needed after a throw
    }

    switch(m_stop_bits)
    {
        case stop_bits::STOP_BITS_1:
            port_settings.c_cflag &= ~CSTOPB ;  // NOLINT (hicpp-signed-bitwise)
            break ;
        case stop_bits::STOP_BITS_2:
            port_settings.c_cflag |= CSTOPB ;
            break ;
        default:
            return -1;
//            throw std::invalid_argument(ERR_MSG_INVALID_STOP_BITS) ;
            // break ; break not needed after a throw
    }

/*

VMIN = 0 and VTIME = 0
    This is a completely non-blocking read - the call is satisfied immediately directly from 
    the driver's input queue. If data are available, it's transferred to the caller's buffer 
    up to nbytes and returned. Otherwise zero is immediately returned to indicate "no data". 
    We'll note that this is "polling" of the serial port, and it's almost always a bad idea. 
    If done repeatedly, it can consume enormous amounts of processor time and is highly 
    inefficient. Don't use this mode unless you really, really know what you're doing. 
VMIN = 0 and VTIME > 0
    This is a pure timed read. If data are available in the input queue, it's transferred to 
    the caller's buffer up to a maximum of nbytes, and returned immediately to the caller. 
    Otherwise the driver blocks until data arrives, or when VTIME tenths expire from the start 
    of the call. If the timer expires without data, zero is returned. A single byte is sufficient 
    to satisfy this read call, but if more is available in the input queue, it's returned to 
    the caller. Note that this is an overall timer, not an intercharacter one. 
VMIN > 0 and VTIME > 0
    A read() is satisfied when either VMIN characters have been transferred to the caller's buffer, 
    or when VTIME tenths expire between characters. Since this timer is not started until the first 
    character arrives, this call can block indefinitely if the serial line is idle. This is the most 
    common mode of operation, and we consider VTIME to be an intercharacter timeout, not an overall 
    one. This call should never return zero bytes read. 
VMIN > 0 and VTIME = 0
    This is a counted read that is satisfied only when at least VMIN characters have been 
    transferred to the caller's buffer - there is no timing component involved. This read can be 
    satisfied from the driver's input queue (where the call could return immediately), or by 
    waiting for new data to arrive: in this respect the call could block indefinitely. We believe 
    that it's undefined behavior if nbytes is less then VMIN. 
*/

    port_settings.c_cc[VMIN] = 1; //static_cast<cc_t>(vmin)
    port_settings.c_cc[VTIME] = 1; //static_cast<cc_t>(vtime) ;

    port_settings.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */

    /* setup for non-canonical mode */
    port_settings.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    port_settings.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    port_settings.c_oflag &= ~OPOST;

    // Apply the modified settings.
    if (tcsetattr(m_ph,TCSANOW, &port_settings) < 0)
    {
        return -1;
//        throw std::runtime_error(std::strerror(errno)) ;
    }
    
    return 0;
}

int serial_port::close()
{
    if(m_ph != INVALID_PORT_HANDLE)
    {
        stop();    
        ::close(m_ph);
    }
    return 0;
}

// cppcheck-suppress unusedFunction
int serial_port::start()
{
    if(m_ph == INVALID_PORT_HANDLE)
    {
        return -1;
    }

    m_exit_flag = false;

    std::thread([=]() {
        std::mutex cvm;
        std::unique_lock<std::mutex> lk(cvm);
    
        for(;;)
        {
            /*bool status =*/ 
            m_cv_exit_flag.wait_for(lk, std::chrono::milliseconds(100), [=]()
                {
                    return m_exit_flag == true;
                }
            );
            if(m_exit_flag == true)
            {
                break;
            }
                        
            std::vector<uint8_t> data = read_all();
            if(data.size() && m_on_read_complete_cb)
            {
//std::cout << "cb: " << data.size() << std::endl;                
                m_on_read_complete_cb(data);
//std::cout << "done" << std::endl;                
            }
        }
#if 0    
		int buf_size = DEFAULT_INPUT_BUF_SIZE;
		std::vector<uint8_t>    input_buf(buf_size);
		std::vector<uint8_t>	total_buf;
		uint8_t *p_data = input_buf.data();	
		int avail_bytes, num_for_read, readed_bytes;

        while(m_future_obj.wait_for(std::chrono::milliseconds(100)) == std::future_status::timeout)
        {
			avail_bytes = 0;
			total_buf.clear();
            m_port_mutex.lock();
			while(1)
			{	
//                std::lock_guard<std::mutex> guard(m_port_mutex);
				int ret = ioctl(m_ph, FIONREAD, &avail_bytes);
				if(ret == -1)
				{
				    if(m_on_error_cb)
				    {
				        m_on_error_cb(errno);
				    }
//				    perror("IOCTL: ");
//                    stop();    
				    break;
				}
				
				if (avail_bytes <= 0) 
				{
					break;
				}
				num_for_read = (avail_bytes > buf_size ? buf_size : avail_bytes);
				
//std::cout << "Data ready for reading: " <<  avail_bytes << std::endl;	
//std::cout << "Try to read: " <<  num_for_read << std::endl;
				readed_bytes = ::read(m_ph, p_data, num_for_read);
				if(readed_bytes == -1)
				{
				    if(m_on_error_cb)
				    {
				        m_on_error_cb(errno);
				    }
				    break;
				}
//std::cout << "Readed: " << readed_bytes << std::endl;
				total_buf.insert(total_buf.end(), p_data, p_data + readed_bytes);
			
			}
            m_port_mutex.unlock();
            if(total_buf.size() && m_on_read_complete_cb)
            {
                m_on_read_complete_cb(total_buf);
            }
//std::cout << total_buf.data() << std::endl;
        }
//std::cout  << "Thread finished" << std::endl;
#endif
    }).detach();

    return 0;
}

int serial_port::stop()
{
    m_exit_flag = true;
    m_cv_exit_flag.notify_all();
   
    return 0;
}

int serial_port::write(const uint8_t *data, int size)
{
    if(m_ph == INVALID_PORT_HANDLE)
    {
        return -1;
    }

    std::lock_guard<std::mutex> guard(m_port_mutex);
 
    int len = ::write(m_ph, data, size);
    if(len == -1 /*&& m_on_error_cb*/)
    {
        if(m_on_error_cb)
        {
            m_on_error_cb(errno);
        }
        perror("serial_port::write");
    }
    
    return len;
}

int serial_port::read(uint8_t *data, int size)
{
    if(m_ph == INVALID_PORT_HANDLE)
    {
        return -1;
    }
    
    std::mutex cvm;
    std::unique_lock<std::mutex> lk(cvm);
    
    int offset = 0;

    for(;offset < (int)size;)
    {
        size_t num_for_read;
        int readed_bytes = 0, avail_bytes = 0;
        /*bool status =*/ 
        m_cv_exit_flag.wait_for(lk, std::chrono::milliseconds(1), [=]()
            {
                return m_exit_flag == true;
            }
        );
        if(m_exit_flag == true)
        {
            break;
        }

        avail_bytes = 0;
        std::lock_guard<std::mutex> guard(m_port_mutex);
        int ret = ioctl(m_ph, FIONREAD, &avail_bytes);
        if(ret == -1)
        {
            if(m_on_error_cb)
            {
                m_on_error_cb(errno);
            }
            break;
        }
//std::cout<< "avail_bytes = " << avail_bytes << std::endl;
        if (avail_bytes <= 0) 
        {
            break;
        }
        num_for_read = (avail_bytes > size ? size : avail_bytes);
//std::cout<< "num_for_read = " << num_for_read << std::endl;

        readed_bytes = ::read(m_ph, data+offset, num_for_read);
        if(readed_bytes == -1)
        {
            if(m_on_error_cb)
            {
                m_on_error_cb(errno);
            }
            break;
        }
        offset += readed_bytes;
	    size -= readed_bytes;
    }

//std::cout<< "offset = " << offset << std::endl;
//std::cout<< "data = " << data << std::endl;

    return offset;
}

const std::vector<uint8_t> serial_port::read_all()
{
    int buf_size = DEFAULT_INPUT_BUF_SIZE;
	std::vector<uint8_t>    input_buf(buf_size);
	std::vector<uint8_t>	total_buf;
	uint8_t *p_data = input_buf.data();	
    std::mutex cvm;
    std::unique_lock<std::mutex> lk(cvm);

    for(;;)
    {
        /*bool status =*/ 
        int size = read(p_data, buf_size);
	if(size > buf_size)
	{
std::cout << "Error: size = " << size << std::endl;
	}
	assert(size <= buf_size);
	    if(size == 0)
	    {
            m_cv_exit_flag.wait_for(lk, std::chrono::milliseconds(10), [=]()
                {
                    return m_exit_flag == true;
                }
            );
            if(m_exit_flag == true)
            {
                break;
            }
	    }
	
        if(size)
        {
//std::cout << "Insert to buf: [" << total_buf.size() << "][" << size << "]: " /*<< p_data*/ << std::endl;
            total_buf.insert(total_buf.end(), p_data, p_data + size);
//            input_buf.
        }
        if(/*size == 0 || */size < buf_size)
        {
            break;
        }
    }
    
//std::cout<< "read_all finished: " << total_buf.size() << std::endl;
    return total_buf;
}

// cppcheck-suppress unusedFunction
const std::string serial_port::read_line()
{
    std::string res;

    if(m_ph == INVALID_PORT_HANDLE)
    {
        return res;
    }

    uint8_t ch;
    std::mutex cvm;
    std::unique_lock<std::mutex> lk(cvm);

    for(;;)
    {
        /*bool status =*/ 
        m_cv_exit_flag.wait_for(lk, std::chrono::milliseconds(10), [=]()
            {
                return m_exit_flag == true;
            }
        );
        if(m_exit_flag == true)
        {
            break;
        }

        if(read(&ch, 1) != 1)
        {
            return res;
        }
        if(ch == '\r')
        {
            read(&ch, 1);
            break;
        }

        res += ch;
    }

    return res;
}

// cppcheck-suppress unusedFunction
void serial_port::purge_input_data() const
{
    tcflush(m_ph, TCIFLUSH);
}

// cppcheck-suppress unusedFunction
void serial_port::purge_output_data() const
{
    tcflush(m_ph, TCOFLUSH);
}

