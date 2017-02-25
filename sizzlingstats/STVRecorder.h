
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef STV_RECORDER_H
#define STV_RECORDER_H

#include "S3Uploader.h"

#include "eiface.h"
#include <cstdint>
#include <time.h>

#include "convar.h"

class CSizzPluginContext;

class CSTVRecorder
{
public:
	CSTVRecorder();
	~CSTVRecorder();

	void Load();
	void Unload( IVEngineServer *pEngine );

	bool StartRecording( CSizzPluginContext *context, const char *szMapName );
	void StopRecording( IVEngineServer *pEngine );

	void LastRecordedDemo( char *path, uint32_t maxPath, char* demo, uint32_t maxDemo ) const;
	void UploadLastDemo( const char *url, CS3UploaderThread *s3uploader );

	static void StvFolderCallback( IConVar *var, const char *pOldValue, float flOldValue );

private:
	static const uint32_t DEMOPATH_MAX_LEN = 128;
	static const uint32_t DEMONAME_MAX_LEN = 128;

	static bool IsValidStvFolder(const char* str);

private:
	ConVarRef m_refTvEnable;

	// "tf/demos"
	char *m_pDemoPath;

	// "adfasfasd.dem"
	char *m_pDemoName;

	bool m_recording;
	bool m_demoToUpload;
};

inline CSTVRecorder::CSTVRecorder():
	m_refTvEnable((IConVar*)NULL),
	m_pDemoPath(NULL),
	m_pDemoName(NULL),
	m_recording(false),
	m_demoToUpload(false)
{
}

inline CSTVRecorder::~CSTVRecorder()
{
}

inline void CSTVRecorder::Load()
{
	m_refTvEnable.Init("tv_enable", false);
	char* mem = new char[DEMOPATH_MAX_LEN + DEMONAME_MAX_LEN];
	m_pDemoPath = mem;
	m_pDemoName = mem + DEMOPATH_MAX_LEN;

	m_pDemoPath[0] = '\0';
	m_pDemoName[0] = '\0';
}

inline void CSTVRecorder::Unload( IVEngineServer *pEngine )
{
	delete [] m_pDemoPath;
	StopRecording(pEngine);

	m_pDemoPath = NULL;
	m_pDemoName = NULL;
	m_recording = false;
	m_demoToUpload = false;
}

inline void CSTVRecorder::StopRecording( IVEngineServer *pEngine )
{
	if (m_recording)
	{
		m_recording = false;
		m_demoToUpload = true;
		pEngine->ServerCommand( "tv_stoprecord\n" );
		pEngine->ServerExecute();
	}
}

inline void CSTVRecorder::LastRecordedDemo( char *path, uint32_t maxPath, char* demo, uint32_t maxDemo ) const
{
	V_strncpy(path, "tf/", maxPath);
	V_strcat(path, m_pDemoPath, maxPath);
	V_strncpy(demo, m_pDemoName, maxDemo);
	V_strcat(demo, ".dem", maxDemo);
}

inline void CSTVRecorder::UploadLastDemo( const char *url, CS3UploaderThread *s3uploader )
{
	if (!m_demoToUpload)
	{
		if (m_recording)
		{
			Msg("[SizzlingStats]: Could not upload STV demo: recording still in progress\n");
		}
		else
		{
			Msg("[SizzlingStats]: Could not upload STV demo: no demo to upload\n");
		}
		return;
	}

	S3UploadInfo_t info = {};

	// get source path and file
	LastRecordedDemo(info.sourceDir, sizeof(info.sourceDir), info.sourceFile, sizeof(info.sourceFile));

	// Set the upload url
	V_strncpy(info.uploadUrl, url, sizeof(info.uploadUrl));

	// put upload info into thread object and start it
	s3uploader->SetUploadInfo(info);
	s3uploader->StartThread();

	m_demoToUpload = false;
}

#endif // STV_RECORDER_H
