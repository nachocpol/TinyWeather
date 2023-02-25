#ifndef PACKETS_H
#define PACKETS_H

#include "stdint.h"

static const uint8_t k_Magic = 123;

enum PacketTypes
{
    REQUEST_TIME = 0,
    DATA
};

typedef struct
{
    uint8_t m_Magic;
    uint8_t m_Type;
}RequestTimePacket;

static const uint8_t k_DataPacketVersion = 0;


typedef struct
{
    uint8_t m_Magic;
    uint8_t m_Type;
    uint8_t m_Version;
    float m_Temperature;
    float m_Humidity;
    float m_Pressure;
}DataPacket;

#endif