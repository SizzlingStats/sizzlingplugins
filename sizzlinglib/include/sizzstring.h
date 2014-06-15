
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#pragma once

#ifndef SIZZ_STRING_H
#define SIZZ_STRING_H

#include <cstdint>
#include "strtools.h"

namespace sizz
{
	// immuatable string with move semantics
	class CString
	{
	public:
		explicit CString( const char *str );
		CString( const char *str, uint32_t length );
		CString( const CString &other );
		CString( CString &&other );
		~CString();

		uint32_t GetLength() const;
		const char *ToCString() const;

	private:
		void CopyString( const char *str );
		void CopyString( const char *str, uint32_t length );

	private:
		char *m_string;
		uint32_t m_length;
	};

	inline CString::CString( const char *str ):
        m_string(nullptr),
        m_length(0)
	{
		CopyString(str);
	}

    inline CString::CString( const char *str, uint32_t length ):
        m_string(nullptr),
        m_length(0)
	{
		CopyString(str, length);
	}

    inline CString::CString( const CString &other ):
        m_string(nullptr),
        m_length(0)
	{
		CopyString(other.m_string, other.m_length);
	}

    inline CString::CString( CString &&other ):
		m_string(other.m_string),
		m_length(other.m_length)
	{
		other.m_string = nullptr;
		other.m_length = 0;
	}

	inline CString::~CString()
	{
		if (m_length > sizeof(m_string)-1)
		{
			free(m_string);
		}
	}

	inline uint32_t CString::GetLength() const
	{
		return m_length;
	}

	inline const char *CString::ToCString() const
	{
		if (m_length > sizeof(m_string)-1)
		{
			return m_string;
		}
		else
		{
			return reinterpret_cast<const char*>(&m_string);
		}
	}

	inline void CString::CopyString( const char *str )
	{
		if (str)
		{
			uint32_t length = static_cast<uint32_t>(V_strlen(str));
			CopyString(str, length);
		}
	}

	inline void CString::CopyString( const char *str, uint32_t length )
	{
		if (str && (length > 0))
		{
			char *dest = nullptr;
			// 3 char or less can fit in the pointer 
			// along with null termination
			if (length < sizeof(m_string))
			{
				dest = reinterpret_cast<char*>(&m_string);
			}
			else
			{
				dest = m_string = reinterpret_cast<char*>(malloc(length+1));
			}
			V_strncpy(dest, str, length+1);
			m_length = length;
		}
	}

	template<class T>
	T &&move( T &param )
	{
		return std::move(param);
	}

}

#endif // SIZZ_STRING_H
