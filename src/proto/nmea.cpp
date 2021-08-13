#include <iostream>
#include <math.h>
//#include <utility>
 
#include "nmea.h"

// cppcheck-suppress unusedFunction
float std::ext::nmea::nmea_to_wgs84(float val)
{
/*  float degree = trunc(val/100);
  float min = (val/100 - degree)*100;
  min = min/60;
  return degree+min;
*/
    int DD = int(val/100); // = int(31.3736664) = 31
    float SS = val - DD * 100; // = 3137.36664 - 3100 = 37.36664

    return DD + SS/60;
}

std::string get_coord_value(float val, const char *fmt)
{
/*
    float a = 3.4;
    float a_frac = a - (int) a;
    float a_int = a - a_frac;
*/
    char buf[16] = {0};
    float int_part = 0.0;
    float fract_part = modf(val, &int_part);
    
    int gg = abs(int_part);
    float mm = fract_part*60.0;

    snprintf(buf, sizeof(buf)-1, /* "%03d%02.5f" */ fmt, gg, mm);
    return std::string(buf);
}

// cppcheck-suppress unusedFunction
std::ext::nmea::coord_pair std::ext::nmea::lat_wgs84_to_nmea(float val)
{
    char h;
    std::string s = get_coord_value(val, "%02d%02.6f");

    h = val > 0 ? 'N' : 'S';
    
    return std::make_pair(s, h);
}

// cppcheck-suppress unusedFunction
std::ext::nmea::coord_pair std::ext::nmea::lon_wgs84_to_nmea(float val)
{
    char h;
    std::string s = get_coord_value(val, "%03d%02.6f");

    h = val > 0 ? 'E' : 'W';

/*    lat_parts = math.modf(float(lat));
    lon_parts = math.modf(float(lon));
    abs_lat = int(abs(lat_parts[1]));
    abs_lon = int(abs(lon_parts[1]));
    m_lat = lat_parts[0] * 60;
    m_lon = lon_parts[0] * 60;
    str_lat = str(abs_lat).zfill(2) + str(m_lat)
    str_lon = str(abs_lon).zfill(3) + str(m_lon)
#    h_lat = lat > 0 ? "N" : "S"
    h_lat = 'N' if float(lat)>0 else 'S'
#    h_lon = lon > 0 ? "E" : "W"
    h_lon = 'E' if float(lon)>0 else 'W'
    return [str_lat, h_lat, str_lon, h_lon]
    */
    return std::make_pair(s, h);
}

    
