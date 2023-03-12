#include "JSONHelper.h"

#include <stdio.h>

JSONObject* JSON_CreateObject(uint32_t maxSize)
{
    JSONObject* object = malloc(sizeof(JSONObject));
    object->m_RawData = malloc(maxSize);
    object->m_DataSize = maxSize;
    object->m_DataPosition = 0;
    return object;
}

void JSON_ReleaseObject(JSONObject* object)
{
    if(object == NULL)
    {
        return;
    }
    free(object->m_RawData);
    free(object);
    object = NULL;
}

void JSON_BeginObject(JSONObject* object)
{
    char* pData = object->m_RawData + object->m_DataPosition;
    object->m_DataPosition += sprintf(pData,"{");
}

void JSON_EndObject(JSONObject* object)
{
    uint32_t writePos = object->m_DataPosition;
    // A bit of a hack... We fixup the comma inserted by last property (if any)
    if(object->m_RawData[writePos - 1] == ',')
    {
        --writePos;
    }
    char* pData = object->m_RawData + writePos;
    object->m_DataPosition += sprintf(pData,"}"); // Do we really need a sprintf...???
}

void JSON_AddProperty_Float(JSONObject* object, const char* name, float value)
{
    if(object == NULL)
    {
        return;
    }
    char* pData = object->m_RawData + object->m_DataPosition;
    object->m_DataPosition += sprintf(pData,"\"%s\" : %f,", name, value);
}

void JSON_AddProperty_U8(JSONObject* object, const char* name, uint8_t value)
{
    if(object == NULL)
    {
        return;
    }
    char* pData = object->m_RawData + object->m_DataPosition;
    object->m_DataPosition += sprintf(pData,"\"%s\" : %u,", name, value);
}