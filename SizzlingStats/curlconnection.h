
#ifndef CURL_CONNECTION_H
#define CURL_CONNECTION_H

#include "curl\curl.h"

// a connection wrapper for curl
class CCurlConnection
{
public:
	enum HttpSendType
	{
		POST = CURLOPT_POST,
		PUT = CURLOPT_UPLOAD,
		GET = CURLOPT_HTTPGET
	};

public:
	CCurlConnection();
	~CCurlConnection();

	void Initialize();
	void Close();

	void ResetOptions();
	
	// curl takes in char *, not const char *
	void SetUrl( char *url );
	void SetHttpSendType( HttpSendType type );


	// this is what curl can handle
	void SetOption( CURLoption opt, long val );
	void SetOption( CURLoption opt, void (*Fn)() );
	void SetOption( CURLoption opt, void *obj );
	void SetOption( CURLoption opt, curl_off_t val );
	
	void Perform();

private:
	CURL *m_pCurl;
};

#endif // CURL_CONNECTION_H