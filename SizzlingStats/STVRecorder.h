
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

	void LastRecordedDemo( char *dest, uint32_t maxlen ) const;

private:
	static const uint32_t DEMONAME_MAX_LEN = 128;

private:
	ConVarRef m_refTvEnable;
	char *m_pDemoName;
	bool m_recording;
	bool m_demoToUpload;
};

inline CSTVRecorder::CSTVRecorder():
	m_refTvEnable((IConVar*)NULL),
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
	m_pDemoName = new char[DEMONAME_MAX_LEN];
}

inline void CSTVRecorder::Unload( IVEngineServer *pEngine )
{
	delete [] m_pDemoName;
	StopRecording(pEngine);
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

inline void CSTVRecorder::LastRecordedDemo( char *dest, uint32_t maxlen ) const
{
	V_strncpy(dest, m_pDemoName, maxlen);
}

#endif // STV_RECORDER_H