////////////////////////////////////////////////////////////////////////////////
// Filename: FtpUploader.cpp
////////////////////////////////////////////////////////////////////////////////

#include "FtpUploader.h"
#include "utlbuffer.h"
#include "curl/curl.h"
#include "PlayerMessage.h"

extern PlayerMessage *g_pMessage;

CFtpUploader::CFtpUploader(const char *ftpUrlInfo)
{
	V_strncpy(m_ftpUrl, ftpUrlInfo, 128);
}

/*CFtpUploader::CFtpUploader(const char *ftpUrlInfo, ...)
{
	va_list argList;
	va_start( argList, ftpUrlInfo );

	V_vsnprintf( m_ftpUrl, 128, ftpUrlInfo, argList );

	va_end( argList );
}*/

CFtpUploader::~CFtpUploader()
{	
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

static int dbgCurl(CURL *curl, curl_infotype type, char *info, size_t, void *)
{
	char temp[128] = "";
	V_snprintf(temp, 128, "%s", info);
	Msg("curl debug says: %s\n", temp);
	g_pMessage->AllUserChatMessage(temp);
	return 0;
}

bool CFtpUploader::UploadFile( CUtlBuffer &buff )
{
	CURL *curl;

	// not thread safe
	//curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, m_ftpUrl);

		/* Define our callback to get called when there's data to be written */ 
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, &CFtpUploader::sendData);

		/* enable uploading */ 
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		/* Set a pointer to our struct to pass to the callback */ 
		curl_easy_setopt(curl, CURLOPT_READDATA, &buff);

		/* create missing dirs on the ftp if not present */
		curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);

		/* Switch on full protocol/debug output */ 
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* Set the size of the file to upload (optional).  If you give a *_LARGE
		option you MUST make sure that the type of the passed-in argument is a
		curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
		make sure that to pass in a type 'long' argument. */ 
		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)buff.GetBytesRemaining());

		/* build a list of commands to pass to libcurl */ 
		struct curl_slist *pHeaderList = NULL;
		pHeaderList = curl_slist_append(pHeaderList, "RNFR asdf.html.uploading");
		pHeaderList = curl_slist_append(pHeaderList, "RNTO asdf.html");

		/* pass in that last of FTP commands to run after the transfer */ 
		curl_easy_setopt(curl, CURLOPT_POSTQUOTE, pHeaderList);

		//curl_easy_setopt ( curl, CURLOPT_DEBUGFUNCTION, &dbgCurl ); 

		CURLcode res = curl_easy_perform(curl);

		/* clean up the FTP commands list */ 
		curl_slist_free_all (pHeaderList);

		/* always cleanup */ 
		curl_easy_cleanup(curl);

		//curl_global_cleanup();

		if(res != CURLE_OK) {
			/* we failed */ 
			Msg( "curl told us %d\n", res );
			return false;
		}
		return true;
	}

	//curl_global_cleanup();
	return false;
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
