#ifndef __STD_EXT_NETWORK_TOOLS_HPP__
#define __STD_EXT_NETWORK_TOOLS_HPP__

#include <vector>
#include <string>

namespace std::ext
{
    namespace network
    {
        std::vector<std::string>  get_ifaces_list();
        std::string               get_iface_ipv4_addr(const char *ifname);
        std::string               get_iface_ipv6_addr(const char *ifname);
        std::string               get_iface_hwaddr(const char *ifname);
    }
}


#endif
