
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

#include "S3Uploader.h"
#include "filesystem.h"
#include "utlbuffer.h"
#include "curlconnection.h"
#include "SizzFileSystem.h"
#include "miniz.h"
#include <memory>

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
	char sourcePath[1024];
	V_strncpy(sourcePath, m_info.sourceDir, sizeof(sourcePath));
	V_strncat(sourcePath, "/", sizeof(sourcePath));
	V_strcat(sourcePath, m_info.sourceFile, sizeof(sourcePath));

	FileHandle_t file = sizzFile::SizzFileSystem::OpenFile(sourcePath, "rb");
	if (!file)
	{
		Msg("could not open demo file %s\n", sourcePath);
		return false;
	}

	off_t fileSize = sizzFile::SizzFileSystem::GetFileSize(file);
	std::unique_ptr<unsigned char[]> fileMem(new unsigned char[fileSize]());
	sizzFile::SizzFileSystem::ReadToMem(fileMem.get(), fileSize, file);
	sizzFile::SizzFileSystem::CloseFile(file);

	mz_zip_archive zip{};
	if (mz_zip_writer_init_heap(&zip, 0, fileSize) != MZ_TRUE)
	{
		return false;
	}

	if (mz_zip_writer_add_mem(&zip, m_info.sourceFile, fileMem.get(), fileSize, MZ_DEFAULT_COMPRESSION) != MZ_TRUE)
	{
		return false;
	}

	void* compressedFile = nullptr;
	size_t compressedSize = 0;
	mz_zip_writer_finalize_heap_archive(&zip, &compressedFile, &compressedSize);

	bool result = SendMemory(compressedFile, compressedSize);
	mz_zip_writer_end(&zip);
	return result;
}

bool CS3Uploader::SendMemory(void* mem, size_t size)
{
	CCurlConnection connection;
	if (!connection.Initialize())
	{
		return false;
	}

	// Place the zip that is in memory into a CUtlBuffer for cURL to use
	CUtlBuffer fileBuff;
	fileBuff.SetExternalBuffer(mem, size, size);

	connection.SetUrl(m_info.uploadUrl);
	connection.SetBodyReadFunction(&sendData);
	connection.SetBodyReadUserdata(&fileBuff);

	// peer verification, enable uploading, http put
	connection.SetOption(CURLOPT_SSL_VERIFYPEER, 0L);
	connection.SetOption(CURLOPT_UPLOAD, 1L);
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
	if (res != CURLE_OK)
	{
		Msg("curl told us %d\n", res);
		return false;
	}
	return true;
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

