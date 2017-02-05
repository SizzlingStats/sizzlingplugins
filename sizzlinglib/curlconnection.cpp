
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "curlconnection.h"

typedef void (*voidFn)();

CCurlConnection::CCurlConnection():
	m_pCurl(NULL),
	m_pHeaderList(NULL)
{
}

CCurlConnection::~CCurlConnection()
{
	Close();
}

CURL *CCurlConnection::Initialize()
{
	m_pCurl = curl_easy_init();
	return m_pCurl;
}

void CCurlConnection::Close()
{
	ClearHeaderList();

	if (m_pCurl)
	{
		curl_easy_cleanup(m_pCurl);
		m_pCurl = NULL;
	}
}

void CCurlConnection::ResetOptions()
{
	// this one requires the check
	if (m_pCurl)
	{
		curl_easy_reset(m_pCurl);
	}
}

void CCurlConnection::SetUrl( char *url )
{
	SetOption(CURLOPT_URL, url);
}

void CCurlConnection::SetHttpSendType( HttpSendType type )
{
	SetOption(static_cast<CURLoption>(type), 1L);
}

void CCurlConnection::SetBodyReadFunction( FnCurlCallback fnRead )
{
	SetOption(CURLOPT_READFUNCTION, reinterpret_cast<voidFn>(fnRead));
}

void CCurlConnection::SetBodyReadUserdata( void *pUserdata )
{
	SetOption(CURLOPT_READDATA, pUserdata);
}

void CCurlConnection::SetHeaderReadFunction( FnCurlCallback fnHeaderRead )
{
	SetOption(CURLOPT_HEADERFUNCTION, reinterpret_cast<voidFn>(fnHeaderRead));
}

void CCurlConnection::SetHeaderReadUserdata( void *pUserdata )
{
	SetOption(CURLOPT_HEADERDATA, pUserdata);
}

void CCurlConnection::SetBodyWriteFunction( FnCurlCallback fnWrite )
{
	SetOption(CURLOPT_WRITEFUNCTION, reinterpret_cast<voidFn>(fnWrite));
}

void CCurlConnection::SetBodyWriteUserdata( void *pUserdata )
{
	SetOption(CURLOPT_WRITEDATA, pUserdata);
}

void CCurlConnection::AddHeader( const char *pszHeader )
{
	m_pHeaderList = curl_slist_append(m_pHeaderList, pszHeader);
}

void CCurlConnection::ClearHeaderList()
{
	curl_slist_free_all(m_pHeaderList);
	m_pHeaderList = NULL;
}

void CCurlConnection::SetOption( CURLoption opt, long val )
{
	curl_easy_setopt(m_pCurl, opt, val);
}

void CCurlConnection::SetOption( CURLoption opt, void (*Fn)() )
{
	if (Fn)
	{
		curl_easy_setopt(m_pCurl, opt, Fn);
	}
}

void CCurlConnection::SetOption( CURLoption opt, void *obj )
{
	if (obj)
	{
		curl_easy_setopt(m_pCurl, opt, obj);
	}
}

void CCurlConnection::SetOption( CURLoption opt, curl_off_t val )
{
	curl_easy_setopt(m_pCurl, opt, val);
}
	
CURLcode CCurlConnection::Perform()
{
	SetOption(CURLOPT_HTTPHEADER, m_pHeaderList);
	return curl_easy_perform(m_pCurl);
}

long CCurlConnection::GetResponseCode()
{
	long responseCode = 0;
	return curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &responseCode);
}

