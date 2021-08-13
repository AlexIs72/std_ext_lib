#ifndef __STD_EXT_NMEA_TOOLS_HPP__
#define __STD_EXT_NMEA_TOOLS_HPP__

//#include <vector>
#include <string>
#include <utility>

namespace std::ext
{
    namespace nmea
    {
        using coord_pair = std::pair<std::string, char>;  
    
        float nmea_to_wgs84(float val);
        coord_pair lat_wgs84_to_nmea(float val);
        coord_pair lon_wgs84_to_nmea(float val);
    }
}


#endif
