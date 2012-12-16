
#ifndef STV_RECORDER_H
#define STV_RECORDER_H

#include "eiface.h"
#include "PlayerMessage.h"
#include <time.h>

#include "convar.h"

class CSTVRecorder
{
public:
	CSTVRecorder();
	~CSTVRecorder();

	void Load();
	void Unload( IVEngineServer *pEngine );

	bool StartRecording( IVEngineServer *pEngine, const char *szMapName );
	void StopRecording( IVEngineServer *pEngine );

private:
	ConVarRef m_refTvEnable;
};

inline CSTVRecorder::CSTVRecorder():
	m_refTvEnable((IConVar*)NULL)
{
}

inline CSTVRecorder::~CSTVRecorder()
{
}

inline void CSTVRecorder::Load()
{
	m_refTvEnable.Init("tv_enable", false);
}

inline void CSTVRecorder::Unload( IVEngineServer *pEngine )
{
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

	// create the record string
	char recordstring[128] = {};
	V_snprintf(recordstring, 128, "tv_record %d%d%d_%d%d_%s\n", year, month, ltime.tm_mday, ltime.tm_hour, ltime.tm_min );
	
	// unload the sourcemod match recorder plugin so we can take over
	pEngine->ServerCommand( "sm plugins unload matchrecorder\n" );

	// turn off stv auto recording by tf2
	pEngine->ServerCommand( "tv_autorecord 0\n" );

	// stop recording the current demo if someone is
	pEngine->ServerCommand( "tv_stoprecord\n" );

	// start recording our demo
	pEngine->ServerCommand( recordstring );
}

inline void CSTVRecorder::StopRecording( IVEngineServer *pEngine )
{
	pEngine->ServerCommand( "tv_stoprecord\n" );
}

#endif // STV_RECORDER_H