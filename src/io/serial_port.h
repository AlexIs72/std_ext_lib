#ifndef __SERIAL_PORT_HPP__
#define __SERIAL_PORT_HPP__

#include <termios.h>

#include <limits>
#include <string>
#include <vector>
#include <future>
#include <mutex>
#include <functional>
#include <condition_variable>

#define INVALID_PORT_HANDLE     0

/*
https://github.com/crayzeewulf/libserial/blob/master/src/SerialPort.cpp
*/

enum class baud_rate : speed_t
{
	BAUD_50      = B50,
	BAUD_75      = B75,
	BAUD_110     = B110,
	BAUD_134     = B134,
	BAUD_150     = B150,
	BAUD_200     = B200,
	BAUD_300     = B300,
	BAUD_600     = B600,
	BAUD_1200    = B1200,
	BAUD_1800    = B1800,
	BAUD_2400    = B2400,
	BAUD_4800    = B4800,
	BAUD_9600    = B9600,
	BAUD_19200   = B19200,
	BAUD_38400   = B38400,
	BAUD_57600   = B57600,
	BAUD_115200  = B115200,
	// ...
	BAUD_DEFAULT = BAUD_115200,
	BAUD_INVALID = std::numeric_limits<speed_t>::max()
};

enum class char_size : tcflag_t
{
	CHAR_SIZE_5       = CS5, // !< 5 bit characters.
	CHAR_SIZE_6       = CS6, // !< 6 bit characters.
	CHAR_SIZE_7       = CS7, // !< 7 bit characters.
	CHAR_SIZE_8       = CS8, // !< 8 bit characters.
	CHAR_SIZE_DEFAULT = CS8, // !< 8 bit characters.
	CHAR_SIZE_INVALID = std::numeric_limits<tcflag_t>::max()
} ;

/**
 * @brief The allowed flow control types.
 */
enum class flow_control : tcflag_t
{
	FLOW_CONTROL_HARDWARE,
	FLOW_CONTROL_SOFTWARE,
	FLOW_CONTROL_NONE,
	FLOW_CONTROL_DEFAULT = FLOW_CONTROL_NONE,
	FLOW_CONTROL_INVALID = std::numeric_limits<tcflag_t>::max()
} ;

enum class parity : tcflag_t
{
	PARITY_EVEN,                                          // !< Even parity.
	PARITY_ODD,                                           // !< Odd parity.
	PARITY_NONE,                                          // !< No parity i.e. parity checking disabled.
	PARITY_DEFAULT = PARITY_NONE,                         // !< No parity i.e. parity checking disabled.
	PARITY_INVALID = std::numeric_limits<tcflag_t>::max() // !< Invalid parity value.
} ;

enum class stop_bits : tcflag_t
{
	STOP_BITS_1,                     // !< 1 stop bit.
	STOP_BITS_2,                     // !< 2 stop bits.
	STOP_BITS_DEFAULT = STOP_BITS_1, // !< 1 stop bit.
	STOP_BITS_INVALID = std::numeric_limits<tcflag_t>::max()
} ;

enum class open_mode : uint8_t
{
    OPEN_MODE_RD = 0,
    OPEN_MODE_RW,
    OPEN_MODE_DEFAULT = OPEN_MODE_RW,
    OPEN_MODE_INVALID = std::numeric_limits<uint8_t>::max()
};

/*
enum class read_mode : uint8_t
{
	READ_MODE_RAW = 0,
	READ_MODE_LINE,
	READ_MODE_DEFAULT = READ_MODE_RAW,
	READ_MODE_INVALID = std::numeric_limits<uint8_t>::max()		
} ;
*/

class serial_port
{
    public:
        serial_port ();
        virtual ~serial_port ();
//        int open(const char *name);     
        int open(const char *name, open_mode om);     
		int close();

		inline void set_baud_rate(baud_rate br)
		{
			m_baud_rate = br;
		}

		inline void set_char_size(char_size cs)
		{
			m_char_size = cs;
		}

		inline void set_parity(parity prt)
		{
			m_parity = prt;
		}

		inline void set_stop_bits(stop_bits sb)
		{
			m_stop_bits = sb;
		}

		inline void set_flow_control(flow_control fc)
		{
			m_flow_control = fc;
		}

        int start();
        int stop();
        
        inline bool is_opened() const
        {
            return (m_ph != INVALID_PORT_HANDLE);
        }

        const std::vector<uint8_t> read_all();
        const std::string read_line();
        
        void purge_input_data() const;
        void purge_output_data() const;
        
        int read(uint8_t *data, int size);
        
        inline int write(const std::string &data)
        {
            return write((const uint8_t*)data.c_str(), data.size());
        }
        
        inline int write(const std::vector<uint8_t> &data)
        {
            return write(data.data(), data.size());
        }
        
        int write(const uint8_t *data, int size);
        

        inline void on_read_complete(std::function<void(const std::vector<uint8_t> &data)> cb)
        {
            m_on_read_complete_cb = cb;
//            m_cb_param = cb_param;
        }

        inline void on_error(std::function<void(int err_code)> cb)
        {
            m_on_error_cb = cb;
//            m_cb_param = cb_param;
        }

    private:

		int set_port_settings();

        int 			m_ph;
//		termios m_port_settings;
//        open_mode       m_open_mode;
		baud_rate		m_baud_rate;
		char_size		m_char_size;
		parity			m_parity;
		stop_bits 		m_stop_bits;
		flow_control 	m_flow_control;

//		std::vector<uint8_t>	m_input_buf;
//        std::promise<void>      m_exit_signal;
//        std::future<void>       m_future_obj;
        bool m_exit_flag;
        std::condition_variable     m_cv_exit_flag;
        std::mutex              m_port_mutex;

        std::function<void(const std::vector<uint8_t> &data)> m_on_read_complete_cb;
        std::function<void(int err_code)> m_on_error_cb;

//		read_mode	m_read_mode;
};

#endif /* end of include guard SERIAL_PORT_HPP */

