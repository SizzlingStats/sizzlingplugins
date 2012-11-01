////////////////////////////////////////////////////////////////////////////////
// Filename: FtpUploader.h
////////////////////////////////////////////////////////////////////////////////
#ifndef FTP_UPLOADER_H
#define FTP_UPLOADER_H

#include "stddef.h"

class CUtlBuffer;

class CFtpUploader
{
public:
	//static void ConnectToFtp();
	static bool UploadFile( const char *ftpUrl, CUtlBuffer &buff );

private:
	static size_t sendData( void *ptr, size_t size, size_t nmemb, void *userdata );
};

#endif // FTP_UPLOADER_H

