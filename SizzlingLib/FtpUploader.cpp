
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: FtpUploader.cpp
////////////////////////////////////////////////////////////////////////////////

#include "FtpUploader.h"
#include "utlbuffer.h"
#include "curlconnection.h"

static int dbgCurl(CURL *curl, curl_infotype type, char *info, size_t, void *)
{
	char temp[128] = "";
	V_snprintf(temp, 128, "%s", info);
	Msg("curl debug says: %s\n", temp);
	return 0;
}

bool CFtpUploader::UploadFile( const char *ftpUrl, CUtlBuffer &buff )
{
	CCurlConnection connection;
	if (connection.Initialize())
	{
		connection.SetUrl(const_cast<char*>(ftpUrl));
		connection.SetBodyReadFunction(&CFtpUploader::sendData);
		connection.SetBodyReadUserdata(&buff);
		
		// enable uploading
		connection.SetOption(CURLOPT_UPLOAD, 1L);
		
		// create missing dirs on the ftp if not present
		connection.SetOption(CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
		
	#ifndef NDEBUG
		connection.SetOption(CURLOPT_VERBOSE, 1L);
	#endif
	
		// Set the size of the file to upload (optional). If you give a *_LARGE
		// option you MUST make sure that the type of the passed-in argument is a
		// curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
		// make sure that to pass in a type 'long' argument.
		connection.SetOption(CURLOPT_INFILESIZE_LARGE, (curl_off_t)buff.GetBytesRemaining());
		
		// build a list of commands to pass to libcurl
		struct curl_slist *pPostCommandList = NULL;
		pPostCommandList = curl_slist_append(pPostCommandList, "RNFR asdf.html.uploading");
		pPostCommandList = curl_slist_append(pPostCommandList, "RNTO asdf.html");

		// pass in that last of FTP commands to run after the transfer
		connection.SetOption(CURLOPT_POSTQUOTE, pPostCommandList);
		
		CURLcode res = connection.Perform();
		
		curl_slist_free_all (pPostCommandList);
		
		if (res != CURLE_OK)
		{
			Msg( "curl told us %d\n", res );
			return false;
		}
		return true;
	}
	else
	{
		return false;
	}
}

size_t CFtpUploader::sendData(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	CUtlBuffer *pBuffer = static_cast<CUtlBuffer*>(userdata);
	const int maxSize = size*nmemb;
	if ( pBuffer->GetBytesRemaining() >= maxSize )
	{
		pBuffer->Get( ptr, maxSize );
		return maxSize;
	}
	else
	{
		const int bytesRemaining = pBuffer->GetBytesRemaining();
		pBuffer->Get( ptr, bytesRemaining );
		return bytesRemaining;
	}
}

/*bool CFtpUploader::AttemptUpload( const char *url, unsigned int crc, CUtlBuffer &buff )
{
	// keeps trying until it gets it right
	// 5 tries max
	int failureCount = 0;	
	while (1)
	{
		if ( UploadFile( url, buff ) )
		{
			return true;
		}
		else
		{
			++failureCount;
			Msg( "failed to download file: %i time(s)\n" );
			if  ( failureCount == 5 )
			{
				Msg( "aborting download due to 5 successive failures\n" );
				return false;
			}
		}
	}
}*/

