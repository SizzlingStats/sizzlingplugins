////////////////////////////////////////////////////////////////////////////////
// Filename: downloader.h
////////////////////////////////////////////////////////////////////////////////
#ifndef DOWNLOADER_H
#define DOWNLOADER_H

typedef unsigned int size_t;
class CUtlBuffer;

class CDownloader
{
public:

	CDownloader(void);
	~CDownloader(void);

	bool		DownloadFile( const char *url, CUtlBuffer &buf );
	bool		DownloadFileAndVerify( const char *url, unsigned int crc, CUtlBuffer &buf );

private:
	static size_t	rcvData( void *ptr, size_t size, size_t nmemb, void *userdata );
	bool			VerifyFile( const unsigned int crc, CUtlBuffer const &buf );
};

#endif // DOWNLOADER_H
