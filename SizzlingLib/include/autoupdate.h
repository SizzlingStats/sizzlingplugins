////////////////////////////////////////////////////////////////////////////////
// Filename: autoupdate.h
////////////////////////////////////////////////////////////////////////////////
#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

#include "downloader.h"
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

// this is for whoever uses the updater thread
// they need to pass in an array of const char *
// and this is the required mapping of array index to data
enum EPluginInfoMap
{
	k_eLocalPluginPath = 0,
	k_ePluginName,
	k_ePluginNameNoExtension,
	k_ePluginExtension,
	k_ePluginDescriptionPart
};

class CAutoUpdater
{
public:
	CAutoUpdater( autoUpdateInfo_t const &info ):
		m_info(info),
		m_downloader(),
		m_bWaitingForUnload(false)
	{
	}

	~CAutoUpdater()
	{
	}

	void Load()
	{
	}

	void PerformUpdateIfAvailable( const char *pUpdateInfo[] );

private:
	// returns true if there is an update available
	bool CheckForUpdate();

	// returns true if v1 is newer than v2
	bool CompareVersions( const char *v1, const char *v2 );

	// wrapper for the filesystem removefile
	inline void	RemoveFile( const char *pRelativePath );

	// unzips the file; maybe have a new class for this?
	//bool 		UnzipFile( const char *pRelativeName, CUtlBuffer &src, CUtlMemory<unsigned char> &dest );
	//unsigned int		Compress( CUtlBuffer &src, CUtlMemory<unsigned char> &dest );

private:
	autoUpdateInfo_t m_info;
	CDownloader m_downloader;
	bool m_bWaitingForUnload;
};

class CAutoUpdateThread: public CThread
{
public:
	CAutoUpdateThread( autoUpdateInfo_t const &info, const char *pUpdateInfo[] ):
		m_autoUpdater(info),
		m_pUpdateInfo(pUpdateInfo)
	{
	}

	virtual ~CAutoUpdateThread()
	{
	}

	void StartThread()
	{
		if (IsAlive())
		{
			Join();
		}
		Start();
	}

	virtual int Run()
	{
		m_autoUpdater.PerformUpdateIfAvailable(m_pUpdateInfo);
		return 0;
	}

	void ShutDown()
	{
		Join();
	}

private:
	CAutoUpdater m_autoUpdater;
	// the plugin will pass static data into here
	const char **m_pUpdateInfo;
};

#endif // AUTOUPDATE_H
