#ifndef __SERIAL_PORT_INFO_HPP__
#define __SERIAL_PORT_INFO_HPP__

#include <vector>

#include "device_info.h"

class serial_port_info : public device_info 
{
    public:
        explicit serial_port_info (const device_info &di);
        virtual ~serial_port_info ();

        static std::vector<serial_port_info> available_ports();    
};

#endif /* end of include guard SERIAL_PORT_INFO_HPP */

