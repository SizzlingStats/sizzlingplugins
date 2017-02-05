
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef CURL_CONNECTION_H
#define CURL_CONNECTION_H

#include "curl/curl.h"

enum HttpStatus : long
{
    HTTP_OK = 200
};

// a connection wrapper for curl
class CCurlConnection
{
public:
	enum HttpSendType
	{
		POST = CURLOPT_POST,
		PUT = CURLOPT_PUT,
		GET = CURLOPT_HTTPGET
	};

	typedef size_t (*FnCurlCallback)(void *ptr, size_t size, size_t nmemb, void *userdata);
	//typedef int (*FnCurlDebugCallback)(CURL*, curl_infotype, char*, size_t, void*);

public:
	CCurlConnection();
	~CCurlConnection();

	CURL *Initialize();
	void Close();

	void ResetOptions();
	
	// curl takes in char *, not const char *
	void SetUrl( char *url );
	void SetHttpSendType( HttpSendType type );

	void SetBodyReadFunction( FnCurlCallback fnRead );
	void SetBodyReadUserdata( void *pUserdata );

	void SetHeaderReadFunction( FnCurlCallback fnHeaderRead );
	void SetHeaderReadUserdata( void *pUserdata );

	void SetBodyWriteFunction( FnCurlCallback fnWrite );
	void SetBodyWriteUserdata( void *pUserdata );

	// this is what curl can handle
	void SetOption( CURLoption opt, long val );
	void SetOption( CURLoption opt, void (*Fn)() );
	void SetOption( CURLoption opt, void *obj );
	void SetOption( CURLoption opt, curl_off_t val );
	
	// can't remove a header, have to destroy the whole list
	// stupid
	void AddHeader( const char *pszHeader );
	void ClearHeaderList();

	CURLcode Perform();
	long GetResponseCode();

private:
	typedef struct curl_slist curl_slist_t;

private:
	CURL *m_pCurl;
	curl_slist_t *m_pHeaderList;
};

#endif // CURL_CONNECTION_H

