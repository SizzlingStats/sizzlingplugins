
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

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

