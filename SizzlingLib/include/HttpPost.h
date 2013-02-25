
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: HttpPost.h
////////////////////////////////////////////////////////////////////////////////
#ifndef HTTP_POST_H
#define HTTP_POST_H

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

class CUtlBuffer;

class CHttpPost
{
public:

	CHttpPost(const char *ftpUrlInfo = NULL);
	~CHttpPost(void);

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

#endif // HTTP_POST_H
