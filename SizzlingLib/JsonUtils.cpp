
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "JsonUtils.h"
#include "utlbuffer.h"

#define END_STATIC_CHAR_CONVERSION( _name, _delimiter, _escapeChar ) \
	}; \
	static CUtlCharConversion _name( _escapeChar, _delimiter, sizeof( s_pConversionArray ## _name ) / sizeof( CUtlCharConversion::ConversionArray_t ), s_pConversionArray ## _name );

BEGIN_CHAR_CONVERSION( s_conv, "\"", '\\' )
	{ '\\', "\\" },
	{ '\"', "\"" }
END_STATIC_CHAR_CONVERSION( s_conv, "\"", '\\' );

// can be a named or unnamed object
CJsonObject::CJsonObject(CUtlBuffer &buff, const char *name /* = NULL */):
	m_buff(buff),
	m_bNeedsComma(false)
{
	if (!m_buff.IsText())
		m_buff.SetBufferType(true,true);

	if (name)
	{
		m_buff.PutDelimitedString(&s_conv, name);
		m_buff.PutString(":");
	}
		
	m_buff.PutString("{");
}

CJsonObject::~CJsonObject()
{
	m_buff.PutString("}");
}

void CJsonObject::InsertKV( const char *key, const char *value )
{
	InsertKey(key);
	m_buff.PutDelimitedString(&s_conv, value);
}

void CJsonObject::InsertKV( const char *key, int value )
{
	InsertKey(key);
	m_buff.PutInt(value);
}

void CJsonObject::InsertKV( const char *key, unsigned int value )
{
	InsertKey(key);
	m_buff.PutUnsignedInt(value);
}

void CJsonObject::InsertKV( const char *key, uint64 value )
{
	InsertKey(key);
	m_buff.Printf( "%llu", value );//.Put(&value, sizeof(uint64));
}

void CJsonObject::InsertKey( const char *key )
{
	if (m_bNeedsComma)
	{
		m_buff.PutString(",");
	}
	else
	{
		m_bNeedsComma = true;
	}
	m_buff.PutDelimitedString(&s_conv, key);
	m_buff.PutString(":");
}

CJsonArray::CJsonArray(CUtlBuffer &buff, const char *name):
	m_buff(buff)
{
	if (!m_buff.IsText())
		m_buff.SetBufferType(true,true);

	m_buff.PutDelimitedString(&s_conv, name);
	m_buff.PutString(":[");
}

CJsonArray::~CJsonArray()
{
	m_buff.PutString("]");
}
