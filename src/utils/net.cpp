/*
uint64_t getIFMAC(const string &ifname) {
  ifstream iface("/sys/class/net/"+ifname+"/address");
  string str((istreambuf_iterator<char>(iface)), istreambuf_iterator<char>());
  if (str.length() > 0) {
    string hex = regex_replace(str, std::regex(":"), "");
    return stoull(hex, 0, 16);
  } else {
    return 0;
  }
} 
*/
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
/*
#ifdef __linux__
    #include <linux/if_link.h>
#elif defined(__APPLE__) or defined (__MACH__) 
//    #include <asdfasdf>
#endif
*/

#include <iostream>
#include <fstream>
 
#include "net.h"

// cppcheck-suppress unusedFunction
std::vector<std::string>  std::ext::network::get_ifaces_list()
{
    std::vector<std::string>    v;

    DIR* dirp = opendir("/sys/class/net");
    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL) {
        if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
        {
            continue;
        }
        v.push_back(dp->d_name);
    }
    closedir(dirp);

    return v;
}

// cppcheck-suppress unusedFunction
std::string std::ext::network::get_iface_ipv4_addr(const char *ifname)
{
    std::string ip_address=""; //"Unable to get IP Address";
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *ifa = NULL;
    int success = 0;
    char host[NI_MAXHOST];

    // retrieve the current interfaces - returns 0 on success
    success = getifaddrs(&interfaces);
    if (success == 0) {
        int /*family, s,*/ n;
        for (ifa = interfaces, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
            if (ifa->ifa_addr == NULL || 
                ifa->ifa_addr->sa_family != AF_INET ||
                strcmp(ifa->ifa_name, ifname) != 0)
            {
                continue;
            }
            if(getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                           host, NI_MAXHOST,
                           NULL, 0, NI_NUMERICHOST) == 0)
            {
                ip_address = host;
            }
        }
#if 0
           for (ifa = interfaces, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
               if (ifa->ifa_addr == NULL)
                   continue;

               family = ifa->ifa_addr->sa_family;

               /* Display interface name and family (including symbolic
                  form of the latter for the common families) */

               printf("%-8s %s (%d)\n",
                      ifa->ifa_name,
                      (family == AF_PACKET) ? "AF_PACKET" :
                      (family == AF_INET) ? "AF_INET" :
                      (family == AF_INET6) ? "AF_INET6" : "???",
                      family);

               /* For an AF_INET* interface address, display the address */

               if (family == AF_INET || family == AF_INET6) {
                   s = getnameinfo(ifa->ifa_addr,
                           (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                                 sizeof(struct sockaddr_in6),
                           host, NI_MAXHOST,
                           NULL, 0, NI_NUMERICHOST);
                   if (s != 0) {
                       printf("getnameinfo() failed: %s\n", gai_strerror(s));
                       exit(EXIT_FAILURE);
                   }

                   printf("\t\taddress: <%s>\n", host);

               } else if (family == AF_PACKET && ifa->ifa_data != NULL) {
                   struct rtnl_link_stats *stats = (struct rtnl_link_stats *)(ifa->ifa_data);

                   printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
                          "\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
                          stats->tx_packets, stats->rx_packets,
                          stats->tx_bytes, stats->rx_bytes);
               }
           }
#endif

#if 0
        // Loop through linked list of interfaces
        temp_addr = interfaces;
        while(temp_addr != NULL) {
std::cout << temp_addr->ifa_name << std::endl;
/*            if(temp_addr->ifa_addr->sa_family == AF_INET) {
                // Check if interface is en0 which is the wifi connection on the iPhone
                if(strcmp(temp_addr->ifa_name, ifname)==0){
                    ip_address=inet_ntoa(((struct sockaddr_in*)temp_addr->ifa_addr)->sin_addr);
                }
            }*/
            temp_addr = temp_addr->ifa_next;
        }
#endif
    }

    // Free memory
    freeifaddrs(interfaces);
    return ip_address;
}

// cppcheck-suppress unusedFunction
std::string std::ext::network::get_iface_ipv6_addr(const char *ifname)
{
    return ifname;
}

// cppcheck-suppress unusedFunction
std::string std::ext::network::get_iface_hwaddr(const char *ifname)
{
    std::string ifname_s("/sys/class/net/");
    ifname_s += ifname;
    ifname_s += "/address";

    std::ifstream iface(ifname_s);

    return std::string (std::istreambuf_iterator<char>(iface), std::istreambuf_iterator<char>());

/*    if (str.length() > 0) {
    string hex = regex_replace(str, std::regex(":"), "");
    return stoull(hex, 0, 16);
  } else {
    return 0;
  }
    return iface_name;
    */
}
