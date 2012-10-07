#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include "platform.h"

class CUtlBuffer;

// can be a named or unnamed object
class CJsonObject
{
public:
	CJsonObject(CUtlBuffer &buff, const char *name = NULL);
	~CJsonObject();

	void InsertKV( const char *key, const char *value );
	void InsertKV( const char *key, int value );
	void InsertKV( const char *key, uint64 value );

private:
	void InsertKey( const char *key );

private:
	CUtlBuffer &m_buff;
	bool m_bNeedsComma;
};

class CJsonArray
{
public:
	CJsonArray(CUtlBuffer &buff, const char *name);
	~CJsonArray();

private:
	CUtlBuffer &m_buff;
};

#endif // JSON_UTILS_H
