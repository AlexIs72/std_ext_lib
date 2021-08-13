#ifndef GNSS_H
#define GNSS_H

#include <stdint.h>
#include <time.h>

typedef enum 
{
    FT_UNKNOWN = -1,
    FT_INACTIVE = 0,
    FT_NOT_AVALIABLE  = 1,
    FT_FIX2D = 2,
    FT_FIX3D = 3,
  //..
    MAX_FIX_TYPE
} fix_type_t;

typedef struct
{
//    time_struct_t utc;
    struct      tm utc;
    bool        is_valid;
    float       lat;                // Широта (знаковая. градусы, доли градуса. )
    float       lon;                // Долгота (знаковая. градусы, доли градуса. )
    float       alt;                // Высота над средним уровнем моря (в метрах)
    float       diff;               // Разница между  эллипсоидом WGS-84 и средним уровнем моря (в метрах)
    float       speed;              // Скорость
    float       direction;          // Курс
    uint8_t     dgps_mode;          // Режим DGPS (A = автономный режим, D = с использованием дифференциальной коррекции
    uint32_t    dgps_age;           // Возраст данных DGPS в секундах
    uint16_t    dgps_sid;           // Номер станции DGPS
    int         sig;                 // Индикатор качества (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive)
    uint8_t     sat_used;           // Количество спутников, используемых в навигационном решении
    uint8_t     sat_visible;
    fix_type_t  fix_type;         // Тип фиксации (1 = Fix not available; 2 = 2D; 3 = 3D
    float       pdop;                // Position Dilution Of Precision
    float       hdop;                // Horizontal Dilution Of Precision
    float       vdop;                // Vertical Dilution Of Precision
} gnss_point_t;


#endif /* end of include guard GNSS_H */

