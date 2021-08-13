#include <iostream>

#include "device_info.h"
#include "serial_port_info.h"

serial_port_info::serial_port_info(const device_info &di) :
    device_info(di)
{
}

serial_port_info::~serial_port_info()
{
}

// cppcheck-suppress unusedFunction
std::vector<serial_port_info> serial_port_info::available_ports() 
{
    std::vector<serial_port_info> serial_list;    
    std::vector<device_info> devs_list = device_info::devices_list("/sys/class/tty");

    for(const device_info &di : devs_list)
    {
        if(!di.is_virtual())
        {
            serial_port_info    spi(di);
            spi.read_device_info();
            serial_list.push_back(spi);
        }
    }

    return serial_list;
}
