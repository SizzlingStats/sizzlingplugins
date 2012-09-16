
#include "curlconnection.h"

CCurlConnection::CCurlConnection():
	m_pCurl(NULL)
{
}

CCurlConnection::~CCurlConnection()
{
}

void CCurlConnection::Initialize()
{
	m_pCurl = curl_easy_init();
}

void CCurlConnection::Close()
{
	curl_easy_cleanup(m_pCurl);
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
	SetOption( CURLOPT_URL, url );
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
	
void CCurlConnection::Perform()
{
	curl_easy_perform(m_pCurl);
}