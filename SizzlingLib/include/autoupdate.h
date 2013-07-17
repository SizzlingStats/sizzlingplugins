
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: autoupdate.h
////////////////////////////////////////////////////////////////////////////////
#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

#include "downloader.h"
#include "threading.h"

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
		m_bWaitingForUnload(false)
	{
	}

	~CAutoUpdater()
	{
	}

	void Load()
	{
	}

	bool PerformUpdateIfAvailable( const char *pUpdateInfo[] );

private:
	// returns true if there is an update available
	bool CheckForUpdate();

	// returns true if v1 is newer than v2
	static bool CompareVersions( const char *v1, const char *v2 );

	// wrapper for the filesystem removefile
	inline void	RemoveFile( const char *pRelativePath );

	// unzips the file; maybe have a new class for this?
	//bool 		UnzipFile( const char *pRelativeName, CUtlBuffer &src, CUtlMemory<unsigned char> &dest );
	//unsigned int		Compress( CUtlBuffer &src, CUtlMemory<unsigned char> &dest );

private:
	autoUpdateInfo_t m_info;
	bool m_bWaitingForUnload;
};

class CAutoUpdateThread: public sizz::CThread
{
public:
	CAutoUpdateThread( autoUpdateInfo_t const &info, const char *pUpdateInfo[] ):
		m_autoUpdater(info),
		m_pUpdateInfo(pUpdateInfo),
		m_finished_callback(nullptr)
	{
	}

	virtual ~CAutoUpdateThread()
	{
	}

	void StartThread()
	{
		Join();
		Start();
	}

	void SetOnFinishedUpdateCallback( std::function<void(bool)> callback )
	{
		m_finished_callback = std::move(callback);
	}

	virtual int Run()
	{
		bool ret = m_autoUpdater.PerformUpdateIfAvailable(m_pUpdateInfo);
		if (m_finished_callback)
		{
			m_finished_callback(ret);
		}
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
	std::function<void(bool)> m_finished_callback;
};

#endif // AUTOUPDATE_H

