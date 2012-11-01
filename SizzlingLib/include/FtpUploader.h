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

	CFtpUploader(const char *ftpUrlInfo = NULL);
	//CFtpUploader(const char *ftpUrlInfo, ...);
	~CFtpUploader(void);

	void	ConnectToFtp(const char *ftpUrl);

	bool	UploadFile( CUtlBuffer &buff );

private:
	CFtpUploader(const CFtpUploader &);

private:
	static size_t	sendData( void *ptr, size_t size, size_t nmemb, void *userdata );
	bool			VerifyFile( const unsigned int crc, CUtlBuffer const &buf );
private:
	char m_ftpUrl[128];
};

#endif // FTP_UPLOADER_H

