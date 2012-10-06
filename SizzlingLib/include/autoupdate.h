////////////////////////////////////////////////////////////////////////////////
// Filename: autoupdate.h
////////////////////////////////////////////////////////////////////////////////
#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

#include "downloader.h"
#include "utlbuffer.h"
#include "threadtools.h"

typedef struct autoUpdateInfo_s
{
	char fileName[256];
	char fileUrl[128];
	char metaUrl[128];
	char destDir[64];
	unsigned int fileCRC;
	char currentVersion[16];
} autoUpdateInfo_t;

class CAutoUpdater
{
public:
	CAutoUpdater( autoUpdateInfo_t const &info ): m_info(info)
	{
	}
	~CAutoUpdater(void)
	{
	}
	
	void		Load()
	{
	}
	void		PerformUpdateIfAvailable( const char *pluginPath,
											 const char *pluginName,
											 const char *pluginNameNoExtension,
											 const char *pluginExtension,
											 const char *pluginDescriptionPart);

private:
	// returns true if there is an update available
	bool		CheckForUpdate();

	// returns true if v1 is newer than v2
	bool		CompareVersions( const char *v1, const char *v2 );

	// wrapper for the filesystem removefile
	inline void		RemoveFile( const char *pRelativePath );
	// unzips the file; maybe have a new class for this?
	//bool 		UnzipFile( const char *pRelativeName, CUtlBuffer &src, CUtlMemory<unsigned char> &dest );

	//unsigned int		Compress( CUtlBuffer &src, CUtlMemory<unsigned char> &dest );

private:
	autoUpdateInfo_t m_info;
	CDownloader m_downloader; // EBCO makes this size 0
};

class CAutoUpdateThread: public CThread
{
public:
	CAutoUpdateThread( autoUpdateInfo_t const &info ): m_autoUpdater(info)
	{
	}

	virtual ~CAutoUpdateThread()
	{
	}

	virtual int Run()
	{
		//m_autoUpdater.PerformUpdateIfAvailable();
	}

	void ShutDown()
	{
		this->Join();
	}

private:
	CAutoUpdater m_autoUpdater;
	CThreadEvent m_EventSignal;
};

#endif // AUTOUPDATE_H
