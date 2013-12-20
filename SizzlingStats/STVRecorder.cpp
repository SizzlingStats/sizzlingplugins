
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "STVRecorder.h"
#include "SizzPluginContext.h"

bool CSTVRecorder::StartRecording( CSizzPluginContext *context, const char *szMapName )
{
	// sourcetv doesn't care if tv_enable is 0 or 1 while the bot is connected.
	// stv will always record if the bot is connected
	if (m_recording || !m_refTvEnable.GetBool() || !context->CanRecordDemo())
	{
		return false;
	}

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
	context->ServerCommand( "sm plugins unload matchrecorder\n" );

	// turn off stv auto recording by tf2
	context->ServerCommand( "tv_autorecord 0\n" );

	// stop recording the current demo if someone is
	context->ServerCommand( "tv_stoprecord\n" );

	// start recording our demo
	context->ServerCommand( cmd );

	m_recording = true;
	m_demoToUpload = false;

	return true;
}
