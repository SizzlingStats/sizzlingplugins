////////////////////////////////////////////////////////////////////////////////
// Filename: downloader.h
////////////////////////////////////////////////////////////////////////////////
#ifndef DOWNLOADER_H
#define DOWNLOADER_H

class CUtlBuffer;

class SizzDownloader
{
public:
	static bool	DownloadFile( const char *url, CUtlBuffer &buf );
	static bool	DownloadFileAndVerify( const char *url, unsigned int crc, CUtlBuffer &buf );

	static bool VerifyFile( const unsigned int actualCRC, CUtlBuffer const &buf );
};

#endif // DOWNLOADER_H
