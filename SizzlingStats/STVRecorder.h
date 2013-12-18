
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef STV_RECORDER_H
#define STV_RECORDER_H

#include "eiface.h"
#include <cstdint>
#include <time.h>

#include "convar.h"
#include "S3Uploader.h"

class CSTVRecorder
{
public:
	CSTVRecorder();
	~CSTVRecorder();

	void Load();
	void Unload( IVEngineServer *pEngine );

	bool StartRecording( IVEngineServer *pEngine, const char *szMapName );
	void StopRecording( IVEngineServer *pEngine );

	void LastRecordedDemo( char *dest, uint32_t maxlen ) const;
	void UploadLastDemo( const char *url, CS3UploaderThread *s3uploader );

private:
	static const uint32_t DEMONAME_MAX_LEN = 128;

private:
	ConVarRef m_refTvEnable;
	char *m_pDemoName;
};

inline CSTVRecorder::CSTVRecorder():
	m_refTvEnable((IConVar*)NULL),
	m_pDemoName(NULL)
{
}

inline CSTVRecorder::~CSTVRecorder()
{
}

inline void CSTVRecorder::Load()
{
	m_refTvEnable.Init("tv_enable", false);
	m_pDemoName = new char[DEMONAME_MAX_LEN];
}

inline void CSTVRecorder::Unload( IVEngineServer *pEngine )
{
	delete [] m_pDemoName;
	StopRecording(pEngine);
}

inline bool CSTVRecorder::StartRecording( IVEngineServer *pEngine, const char *szMapName )
{
	// get the time as an int64
	time_t t = time(NULL);

	// convert it to a struct of time values
	struct tm ltime = *localtime(&t);

	// normalize the year and month
	uint32 year = ltime.tm_year + 1900;
	uint32 month = ltime.tm_mon + 1;

	// construct the demo file name
	V_snprintf(m_pDemoName, DEMONAME_MAX_LEN, "%d%d%d_%d%d_%s", year, month, ltime.tm_mday, ltime.tm_hour, ltime.tm_min, szMapName );
	
	// create the record string
	char cmd[10 + DEMONAME_MAX_LEN + 1] = {};
	V_snprintf(cmd, sizeof(cmd), "tv_record %s\n", m_pDemoName);

	// unload the sourcemod match recorder plugin so we can take over
	pEngine->ServerCommand( "sm plugins unload matchrecorder\n" );

	// turn off stv auto recording by tf2
	pEngine->ServerCommand( "tv_autorecord 0\n" );

	// stop recording the current demo if someone is
	pEngine->ServerCommand( "tv_stoprecord\n" );

	// start recording our demo
	pEngine->ServerCommand( cmd );

	return true;
}

inline void CSTVRecorder::StopRecording( IVEngineServer *pEngine )
{
	pEngine->ServerCommand( "tv_stoprecord\n" );
	pEngine->ServerExecute();
}

inline void CSTVRecorder::LastRecordedDemo( char *dest, uint32_t maxlen ) const
{
	V_strncpy(dest, m_pDemoName, maxlen);
}

inline void CSTVRecorder::UploadLastDemo( const char *url, CS3UploaderThread *s3uploader )
{
	S3UploadInfo_t info = {};

	// set the source path
	V_strcat(info.sourceDir, "tf/", sizeof(info.sourceDir));

	// get source file
	LastRecordedDemo(info.sourceFile, sizeof(info.sourceFile));
	V_strcat(info.sourceFile, ".dem", sizeof(info.sourceFile));

	// Set the upload url
	V_strncpy(info.uploadUrl, url, sizeof(info.uploadUrl));

	// put upload info into thread object and start it
	s3uploader->SetUploadInfo(info);
	s3uploader->StartThread();
}

#endif // STV_RECORDER_H