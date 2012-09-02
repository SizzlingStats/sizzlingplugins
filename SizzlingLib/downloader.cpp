////////////////////////////////////////////////////////////////////////////////
// Filename: downloader.cpp
////////////////////////////////////////////////////////////////////////////////

#include "downloader.h"
#include "checksum_crc.h"
#include "utlbuffer.h"
#include "curl/curl.h"

// TODO: get rid of curl and use sockets @_@

CDownloader::CDownloader()
{
}

CDownloader::~CDownloader()
{	
}

size_t CDownloader::rcvData(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	CUtlBuffer *pBuffer = static_cast<CUtlBuffer*>(userdata);
	pBuffer->Put( ptr, size*nmemb );
	//Msg( (char*)ptr ); // up to 989 characters each time
	return size * nmemb;
}

// should the downloader keep trying if it keeps failing to dl the file?
bool CDownloader::DownloadFile( const char *url, CUtlBuffer &buf )
{
	CURL *curl;

	//curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		/* Define our callback to get called when there's data to be written */ 
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CDownloader::rcvData);
		/* Set a pointer to our struct to pass to the callback */ 
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

		/* Switch on full protocol/debug output */ 
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		CURLcode res = curl_easy_perform(curl);

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

bool CDownloader::DownloadFileAndVerify( const char *url, unsigned int crc, CUtlBuffer &buf )
{
	// keeps trying until it gets it right
	// 5 tries max
	int failureCount = 0;	
	while (1)
	{
		buf.Clear();
		if ( DownloadFile( url, buf ) && VerifyFile( crc, buf ) )
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
}

bool CDownloader::VerifyFile( const unsigned int actualCRC, CUtlBuffer const &buf )
{
	CRC32_t crc = CRC32_ProcessSingleBuffer( buf.PeekGet(), buf.GetBytesRemaining() );

	if ( crc == actualCRC )
	{	
		return true;
	}
	else
	{
		Msg( "file crc: %x, actual crc: %x\n", crc, actualCRC );
		return false;
	}
}
