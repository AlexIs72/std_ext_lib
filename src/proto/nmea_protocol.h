#ifndef __NMEA_PROTOCOL_HPP__
#define __NMEA_PROTOCOL_HPP__

#include <vector>

#include "gnss.h"

typedef struct
{
    uint8_t     id;         // PRN спутника
    uint8_t     elv;        // Возвышение над горизонтом в градусах (от 0 до 90)
    uint16_t    azimuth;    // Азимут от истинного севера, в градусах (от 0 до 359)
    uint8_t     sig;        // Уровень сигнала dB (от 0 до 99)
} sat_info_t;

typedef struct
{
    uint8_t         inview;                 // Количество видимых спутников для данной ГНСС
    std::vector<sat_info_t>	sv_info;   // Информация о видимых спутниках для данной ГНСС
} sat_list_t;

class nmea_protocol
{
    public:
        nmea_protocol ();
        virtual ~nmea_protocol ();

        bool parse(const char *data);
	
		inline void debug(bool state = true)
		{
			m_debug = state;
		}

		inline const gnss_point_t *gnss_point() const
		{
			return &m_curr_state;
		}

    private:
		bool	m_debug;
        gnss_point_t    m_curr_state;
        std::vector<int> m_sat_prn;
		sat_list_t		m_gps_sat_list;
		sat_list_t		m_glonass_sat_list;

        bool check_crc(const std::string &line) const;
        bool parse_gxrmc(const std::vector<std::string> &items);
		bool parse_gxgga(const std::vector<std::string> &items);            
        bool parse_gxgsa(const std::vector<std::string> &items);
		bool parse_gxgsv(const std::vector<std::string> &items, sat_list_t &sat_list);
};

#endif /* end of include guard NMEA_PROTOCOL_HPP */

