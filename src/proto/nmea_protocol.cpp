#include <memory.h>
#include <math.h>

#include <iostream>
#include <algorithm>


#include "string.h"
#include "nmea.h"

#include "nmea_protocol.h"


//#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
//#endif
/*
private string DD2NMEA(double lat, double lng)
{
string nmea = "";
double lata = Math.Abs(lat);
double latd = Math.Truncate(lata);
double latm = (lata - latd) * 60;
string lath = lat > 0 ? "N" : "S";
double lnga = Math.Abs(lng);
double lngd = Math.Truncate(lnga);
double lngm = (lnga - lngd) * 60;
string lngh = lng > 0 ? "E" : "W";

nmea += latd.ToString("00") + latm.ToString("00.00000") + "," + lath + ",";
nmea += lngd.ToString("000") + lngm.ToString("00.00000") + "," + lngh;

return nmea;
}
*/

#if 0
static float nmea_to_wgs84(float val)
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
#endif

/*
 *
 *  http://www.radioscanner.ru/info/article166/
 *
 * */

nmea_protocol::nmea_protocol() :
//    m_is_valid(false),
	m_debug(false)
{
    memset(&m_curr_state, 0, sizeof(gnss_point_t));
}

nmea_protocol::~nmea_protocol()
{
}

bool nmea_protocol::parse(const char *data)
{
    bool is_gprmc_found = false;    
    std::vector<std::string> lines = std::ext::string::split(data, '\n');
	std::vector<std::string> skip_list = {"$GPNON", "$GPVTG", "$GNVTG", "$GNTXT", "$GNGLL"};
//	std::vector<std::string> skip_list = {"$GPNON", "$GPVTG", "$GNVTG", "$GNGLL"};
	std::vector<std::string> gxgga = {"$GPGGA", "$GNGGA"};
	std::vector<std::string> gxgsa = {"$GPGSA", "$GLGSA", "$GNGSA"};
	std::vector<std::string> gxrmc = {"$GPRMC", "$GNRMC"};

//    std::cout << "Found " << lines.size() << " lines" << std::endl;    

    memset(&m_curr_state, 0, sizeof(gnss_point_t));
	m_sat_prn.clear();

	m_gps_sat_list.inview = m_glonass_sat_list.inview = 0;
	m_gps_sat_list.sv_info.clear();
	m_glonass_sat_list.sv_info.clear();

    for(const std::string &s : lines)
    {
        std::string line = std::ext::string::rtrim(s);
//		if(m_debug)
//		{
//			std::cout << "line: " << line << std::endl;
//		}
        if(line.empty() || line.at(0) != '$')
        {
            continue;
        }

		if(std::find(skip_list.begin(), skip_list.end(), line) != skip_list.end())
        {
            // skip
            continue;
        }
        
        if(!check_crc(line))
        {
			if(m_debug)
			{
            	std::cout << "CRC error: " << line << std::endl;
			}
            continue;
        }

		std::string trimmed_line = line;
		size_t pos = line.find("*");
		if(pos != std::string::npos)
		{
			trimmed_line = line.substr(0, pos);
		}
		std::vector<std::string> items = std::ext::string::split(trimmed_line, ',');

//std::cout << "line: " << line << std::endl;
/*        if(items[0] == "$GNTXT")
        {
            //$GNTXT,01,01,02,---------> START TRACK #1*7D
            if(items[4].find("START TRACK") != std::string::npos)
            {
                std::cout << items[4] << std::endl;
            }
        }
        else */if(std::find(gxgga.begin(), gxgga.end(), items[0]) !=  gxgga.end())
        {
			parse_gxgga(items);
        }
        else if(std::find(gxgsa.begin(), gxgsa.end(), items[0]) != gxgsa.end())
        {
            parse_gxgsa(items); 
        }
        else if(items[0] == "$GPGSV")
        {
			parse_gxgsv(items, m_gps_sat_list);
        }
        else if(items[0] == "$GLGSV")
        {
            parse_gxgsv(items, m_glonass_sat_list);
        }
        else if(std::find(gxrmc.begin(), gxrmc.end(), items[0]) != gxrmc.end())
        {
            is_gprmc_found = true;
            parse_gxrmc(items);
        }
        else
        {
			if(m_debug)
			{
            	std::cout << "Invalid string: " << line << std::endl;
			}
        }
    }

    if(is_gprmc_found == false)
    {
        return false;
    }

    // После парсинга подсчитываем количество используемых спутников
	m_curr_state.sat_used = m_sat_prn.size();
	m_curr_state.sat_visible = m_glonass_sat_list.inview + m_gps_sat_list.inview;

    return true; 
}

bool nmea_protocol::check_crc(const std::string &line) const
{
    const uint8_t *p = (const uint8_t *)line.c_str();
    size_t  size = line.size();
    uint8_t crc = 0;

    for(size_t i=1; i<size; i++)
    {
        if(p[i] == '*')
        {
            if(i+2 > size-1 )  // crc string too short; it's correct
            {
                return true;
            }
            uint8_t packet_crc = strtol((const char *)(p+i+1), 0, 16);
            return crc == packet_crc;
        }
        crc ^= p[i];
    }

    return true; // seems no crc; it's valid too
}

bool nmea_protocol::parse_gxgga(const std::vector<std::string> &items)
{
/*
 *  For GNGGA
 *
 *  https://ava.upuaut.net/?p=768
 *
 *  https://www.gpsinformation.org/dale/nmea.htm#GGA
 * */
//std::cout << "parse_gxgga: " << std::endl;
/*
GGA - GPS Данные о местоположении
1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
$GPGGA, hhmmss.ss, 1111.11, a, yyyyy.yy, a, x, xx, x.x, xxx, M, x.x, M, x.x, xxxx*hh
1. Гринвичское время на момент определения местоположения.
2. Географическая широта местоположения.
3. Север/Юг (N/S).
4. Географическая долгота местоположения.
5. Запад/Восток (E/W).
6. Индикатор качества GPS сигнала:
0 = Определение местоположения не возможно или не верно;
1 = GPS режим обычной точности, возможно определение местоположения;
2 = Дифференциальный GPS режим, точность обычная, возможно определение местоположения;
3 = GPS режим прецизионной точности, возможно определение местоположения.
7. Количество используемых спутников (00-12, может отличаться от числа видимых).
8. Фактор Ухудшения Точности Плановых Координат (HDOP).
9. Высота антенны приёмника над/ниже уровня моря.
10. Единица измерения высоты расположения антенны, метры.
11. Геоидальное различие - различие между земным эллипсоидом WGS-84 и уровнем моря(геоидом), ”-” = уровень моря ниже эллипсоида.
12. Единица измерения различия, метры.
13. Возраст Дифференциальных данных GPS - Время в секундах с момента последнего SC104 типа 1 или 9 обновления, заполнено нулями, 
    если дифференциальный режим не используется.
14. Индификатор станции, передающей дифференциальные поправки, ID, 0000-1023.
15. Контрольная сумма строки.
Пример сообщения:
$GPGGA,004241.47,5532.8492,N,03729.0987,E,1,04,2.0 ,-0015,M,,,,*31 
*/

	// items[1] skip
	// items[2] skip
	// items[3] skip
	// items[4] skip
	// items[5] skip

    if(items.size() < 14)
    {
//        std::cout << "GxGGA: Invalid count of arguments: " << items.size() << ", expected - 14" << std::endl;
        return false;
    }

    m_curr_state.sig = (items[6].empty() ? 0 : std::stoi(items[6]));
    m_curr_state.sat_used = (items[7].empty() ? 0 : std::stoi(items[7]));
    // items[8] skip
//std::cout << "9: " << items[9] << std::endl;
    m_curr_state.alt = (items[9].empty() ? 0.0 : std::stof(items[9]));
    // items[10] skip  - M
//    m_curr_state.diff = (items[11].empty() ? 0.0 : std::stof(items[11]));
    // items[12] skip - M
//    std::cout << "items[13] = " << items[13] << std::endl;
//    m_curr_state.dgps_age = (items[13].empty() ? 0 : std::stoi(items[13]));
//    std::cout << "items[14] = " << items[14] << std::endl;
//    m_curr_state.dgps_sid = (items[13].empty() ? 0 : std::stoi(items[13]));

//std::cout << "ok" << std::endl;

	return true;
}

/*
RMC – pекомендуемый минимум GPS / навигационных данных
1 2 3 4 5 6 7 8 9 10 11 12
$GPRMC, Hhmmss.ss, A, 1111.11, A, yyyyy.yy, a, x.x , x.x, ddmmyy, x.x, A *hh <CR><LF>
1. Время фиксации местоположения UTC
2. Состояние: А = действительный, V = предупреждение навигационного приёмника
3,4. Географическая широта местоположения, Север/Юг
5,6. Географическая долгота местоположения, Запад/Восток (E/W)
7. Скорость над поверхностью (SOG) в узлах
8. Истинное направление курса в градусах
9. Дата: dd/mm/yy
10. Магнитное склонение в градусах
11. Запад/Восток (E/W)
12. Контрольная сумма строки (обязательно)
Пример сообщения:
$GPRMC,113650.0, A,5548.607,           N,03739.387,            E,000.01,   255.6,210403,   08.7, E*69 
$GNRMC,061818.00,A,5336.84197339999983,N,08131.400547599999697,E,3.2353826,262.9,100420.00,003.1,W*41
*/
bool nmea_protocol::parse_gxrmc(const std::vector<std::string> &items)
{
    char NS = 'N';
    char EW = 'E';

    if(items.size() < 12)
    {
//        std::cout << "GxGGA: Invalid count of arguments: " << items.size() << ", expected - 14" << std::endl;
        return false;
    }
	
	sscanf(items[1].c_str(), "%2d%2d%2d", 
	                &m_curr_state.utc.tm_hour, 
	                &m_curr_state.utc.tm_min, 
	                &m_curr_state.utc.tm_sec);

	m_curr_state.is_valid = (items[2] == "A");
    m_curr_state.lat = 0.0;
    if(!items[3].empty())
    {
	    m_curr_state.lat = std::ext::nmea::nmea_to_wgs84(std::stof(items[3]));
    }

	NS = (!items[4].empty() ? *(items[4].c_str()) : NS);

    m_curr_state.lon = 0.0;
    if(!items[5].empty())
    {
	    m_curr_state.lon = std::ext::nmea::nmea_to_wgs84(std::stof(items[5]));
    }

	EW = (!items[6].empty() ? *(items[6].c_str()) : EW);

    if(NS == 'S') 
	{
		m_curr_state.lat = m_curr_state.lat*(-1);
	}
     
    if(EW == 'W') 
	{
		m_curr_state.lon = m_curr_state.lon*(-1);
	}

	m_curr_state.speed = (items[7].empty() ? 0.0 : std::stof(items[7]) * 1.852);
	m_curr_state.direction = (items[8].empty() ? 0.0 : std::stof(items[8]));
	sscanf(items[9].c_str(), "%2d%2d%2d", 
	                        &m_curr_state.utc.tm_mday, 
	                        &m_curr_state.utc.tm_mon, 
	                        &m_curr_state.utc.tm_year);
	m_curr_state.utc.tm_mon--;
	m_curr_state.utc.tm_year += (2000-1900);
	// items[10] skip
	// items[11] skip 
	if(items.size() > 12)
	{
	    m_curr_state.dgps_mode = *(items[12].c_str());	
    }

    return true;
}

bool nmea_protocol::parse_gxgsa(const std::vector<std::string> &items)
{
/*
GSA - GPS факторы точности и активные спутники
В этом сообщении отображается режим работы GPS приёмника, параметры спутников, используемых при 
решении навигационной задачи, результаты которой отображены в сообщении $GPGGA и значения 
факторов точности определения координат.
1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18
$GPGSA, a, x, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, x.x, x.x, x.x*hh <CR><LF>
1. Режим: M = Ручной, принудительно включен 2D или 3D режим;
A = Автоматический, разрешено автомат. выбирать 2D/3D.
2. Режим: 1 = Местоположение не определено, 2 = 2D, 3 = 3D
3-14. PRN номера спутников, использованных при решении задачи местоопределения (нули для неиспользованных).
15. Фактор PDOP.
16. Фактор HDOP.
17. Фактор VDOP.
18. Контрольная сумма строки.
Пример сообщения:
$GPGSA,A,3,01,02,03,04,,,,,,,,,2.0,2.0,2.0*34

*/
    if(items.size() < 17)
    {
//        std::cout << "GxGSA: Invalif count of arguments: " << items.size() << ", expected - 17" << std::endl;
        return false;
    }

	// items[1] skip
	m_curr_state.fix_type = (items[2].empty() ? FT_NOT_AVALIABLE : (fix_type_t)std::stoi(items[2]));
	for(int i=3; i<15; i++)
	{
		if(!items[i].empty())
		{
			m_sat_prn.push_back(std::stoi(items[i]));
		}
	}
//std::cout << "PDOP = " << items[15] << " : " << std::stof(items[15]) << std::endl;
	m_curr_state.pdop = (items[15].empty() ? 0 : std::stof(items[15]));
//std::cout << "HDOP = " << items[16] << " : " << std::stof(items[16]) << std::endl;
	m_curr_state.hdop = (items[16].empty() ? 0 : std::stof(items[16]));
//std::cout << "VDOP = " << items[17] << " : " << std::stof(items[17]) << std::endl;
	m_curr_state.vdop = (items[17].empty() ? 0 : std::stof(items[17]));

    return true;
}

bool nmea_protocol::parse_gxgsv(const std::vector<std::string> &items, sat_list_t &sat_list)
{
/*
GSV - видимые спутники GPS
В этом сообщении отображается число видимых спутников(SV), PRN номера этих спутников, 
их высота над местным горизонтом, азимут и отношение сигнал/шум. В каждом сообщении может быть 
информация не более чем о четырех спутниках, остальные данные могут быть расположены в следующих 
по порядку $GPGSV сообщениях. Полное число отправляемых сообщений и номер текущего сообщения 
указаны в первых двух полях каждого сообщения.
1 2 3 4 5 6 7 8 15 16 17 18 19 20
$GPGSV, x, x, xx, xx, xx, xxx, xx..........., xx, xx, xxx, xx*hh <CR><LF>
1. Полное число сообщений, от 1 до 9.
2. Номер сообщения, от 1 до 9.
3. Полное число видимых спутников.
4. PRN номер спутника.
5. Высота, градусы, (90° - максимум).
6. Азимут истинный, градусы, от 000° до 359°.
7. Отношение сигнал/шум от 00 до 99 дБ, ноль - когда нет сигнала.
8-11. Тоже, что в 4-7 для второго спутника.
12-15. Тоже, что в 4-7 для третьего спутника.
16-19. Тоже, что в 4-7 для четвертого спутника.
20. Контрольная сумма строки.
Пример сообщения:
$GPGSV,3,1,12,02,86,172,,09,62,237,,22,39,109,,27, 37,301,*7A
$GPGSV,3,2,12,17,28,050,,29,21,314,,26,18,246,,08, 10,153,*7F
$GPGSV,3,3,12,07,08,231,,10,08,043,,04,06,170,,30, 00,281,*77

*/
	// items[1] skip
	// items[2] skip
	int msgs_count = std::stoi(items[1]);
	int curr_msg = std::stoi(items[2]);
	sat_list.inview = (items[3].empty() ? 0 : std::stoi(items[3]));

    if(items.size() < 19)
    {
//        std::cout << "GxGGA: Invalid count of arguments: " << items.size() << ", expected - 14" << std::endl;
        return false;
    }

    if(sat_list.inview == 0)
    {
        return true;
    }

	if(curr_msg == 1)
	{
		 sat_list.sv_info.clear();
	}

	int rest = (sat_list.inview > 4 ? 4 : sat_list.inview);
	
	if(curr_msg == msgs_count && sat_list.inview > 4)	
	{
		rest = 4-(msgs_count * 4)%((int)sat_list.inview);
	}
	for(int i=0; i<rest; i++)
	{
		sat_info_t si = {};
		si.id = (items[4+i*4].empty() ? 0 : std::stoi(items[4+i*4]));
		si.elv = (items[5+i*4].empty() ? 0 : std::stoi(items[5+i*4]));
		si.azimuth = (items[6+i*4].empty() ? 0 : std::stoi(items[6+i*4]));
        if((size_t)(7+i*4) >= items.size())
        {
            si.sig = 0;
        }
        else
        {
		    si.sig = (items[7+i*4].empty() ? 0 : std::stoi(items[7+i*4]));
        }

		sat_list.sv_info.push_back(si);
	}

	return true;
}

