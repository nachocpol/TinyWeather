#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <stdint.h>

typedef struct
{
    char* m_RawData;
    uint32_t m_DataSize;
    uint32_t m_DataPosition;
}JSONWriter;

JSONWriter* JSON_CreateWriter(uint32_t maxSize);
void JSON_ReleaseWriter(JSONWriter* object);

void JSON_BeginObject();
void JSON_EndObject(JSONWriter* object);

void JSON_AddProperty_Float(JSONWriter* object, const char* name, float value);
void JSON_AddProperty_U8(JSONWriter* object, const char* name, uint8_t value);

#endif