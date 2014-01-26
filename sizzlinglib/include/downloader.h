
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

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
