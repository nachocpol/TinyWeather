#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <stdint.h>

typedef struct
{
    char* m_RawData;
    uint32_t m_DataSize;
    uint32_t m_DataPosition;
}JSONObject;

JSONObject* JSON_CreateObject(uint32_t maxSize);
void JSON_ReleaseObject(JSONObject* object);

void JSON_BeginObject();
void JSON_EndObject(JSONObject* object);

void JSON_AddProperty_Float(JSONObject* object, const char* name, float value);
void JSON_AddProperty_U8(JSONObject* object, const char* name, uint8_t value);

#endif