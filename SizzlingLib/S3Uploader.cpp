
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: S3Uploader.cpp
////////////////////////////////////////////////////////////////////////////////

#include "filesystem.h"
#include "utlbuffer.h"
#include "curlconnection.h"
#include "SizzFileSystem.h"
#include "S3Uploader.h"
#include "zip/xzip.h"

static int dbgCurl(CURL *curl, curl_infotype type, char *info, size_t, void *)
{
	char temp[128] = "";
	V_snprintf(temp, 128, "%s", info);
	Msg("curl debug says: %s\n", temp);
	return 0;
}

static size_t sendData(void *ptr, size_t size, size_t nmemb, void *userdata)
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

bool CS3Uploader::UploadFile()
{
	CCurlConnection connection;
	if (connection.Initialize())
	{
		FileHandle_t file = sizzFile::SizzFileSystem::OpenFile(m_info.sourcePath, "rb");
		if (file)
		{
			//Read the file into memory
			size_t size = sizzFile::SizzFileSystem::GetFileSize(file);
			unsigned char *pMem = new unsigned char[size]();
			sizzFile::SizzFileSystem::ReadToMem(pMem, size, file);
			sizzFile::SizzFileSystem::CloseFile(file);
			
			//create zip archive in memory
			unsigned char *pZip = new unsigned char[size]();
			HZIP hz = CreateZip(pZip, size, ZIP_MEMORY);

			//add demo file to the zip
			ZipAdd(hz,m_info.sourcePath+3, pMem, size, ZIP_MEMORY);
			
			//get start loc of zip in memory and length of compressed zip
			unsigned long ziplength;
			void* zipstart;
			ZipGetMemory(hz, &zipstart, &ziplength);

			//Place the file that is in memory into a CUtlBuffer, which cURL needs
			CUtlBuffer fileBuff;
			fileBuff.AssumeMemory(zipstart, ziplength, ziplength, CUtlBuffer::READ_ONLY);
			CloseZip(hz);

			connection.SetUrl(const_cast<char*>(m_info.uploadUrl));
			connection.SetBodyReadFunction(&sendData);
			connection.SetBodyReadUserdata(&fileBuff);

			// should probably fix having to do this
			connection.SetOption(CURLOPT_SSL_VERIFYPEER, 0L);

			// enable uploading
			connection.SetOption(CURLOPT_UPLOAD, 1L);

			// HTTP PUT please 
			connection.SetOption(CURLOPT_PUT, 1L);
		
		#ifndef NDEBUG
			connection.SetOption(CURLOPT_VERBOSE, 1L);
		#endif
	
			// Set the size of the file to upload (optional). If you give a *_LARGE
			// option you MUST make sure that the type of the passed-in argument is a
			// curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
			// make sure that to pass in a type 'long' argument.
			connection.SetOption(CURLOPT_INFILESIZE_LARGE, (curl_off_t)fileBuff.GetBytesRemaining());

			CURLcode res = connection.Perform();
			
			connection.Close();

			if (res != CURLE_OK)
			{
				Msg( "curl told us %d\n", res );
				return false;
			}
			return true;
		}
		else
		{
			Msg( "could not open demo file %s\n", m_info.sourcePath);
			return false;
		}
	}
	else
	{
		return false;
	}
}

/*bool CS3Uploader::AttemptUpload( const char *url, unsigned int crc, CUtlBuffer &buff )
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

