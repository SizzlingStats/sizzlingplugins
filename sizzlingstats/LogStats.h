
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#pragma once

#ifndef LOG_STATS_H
#define LOG_STATS_H

#include "igameevents.h"

class IGameEventManager2;
class IVEngineServer;
struct edict_t;
class IGameEvent;
class CPluginContext;
struct playerInfo;
class CSizzPluginContext;

class CLogStats: public IGameEventListener2
{
public:
	CLogStats();
	~CLogStats();

	bool Load( CSizzPluginContext &plugin_context );
	void Unload();

	void LevelInit( const char *pMapName );

	void ClientActive( int ent_index );
	void ClientDisconnect( int ent_index );

	void TournamentMatchStarted( const char *RESTRICT hostname, 
								const char *RESTRICT mapname, 
								const char *RESTRICT bluname, 
								const char *RESTRICT redname );
	void TournamentMatchEnded();
	void PreRoundFreezeStarted( bool bTournamentModeOn );

	virtual void FireGameEvent( IGameEvent *event );

private:
	void WriteLog( const char *msg );

private:
	CSizzPluginContext *m_context;
	playerInfo *m_entIndexToPlayerInfo;
};

class CNullLogStats
{
public:
	CNullLogStats() {}
	~CNullLogStats() {}

	bool Load( CSizzPluginContext &plugin_context ) { return true; }
	void Unload() {}

	void LevelInit( const char *pMapName ) {}

	void ClientActive( int ent_index ) {}
	void ClientDisconnect( int ent_index ) {}

	void TournamentMatchStarted( const char *RESTRICT hostname, 
								const char *RESTRICT mapname, 
								const char *RESTRICT bluname, 
								const char *RESTRICT redname ) {}
	void TournamentMatchEnded() {}
	void PreRoundFreezeStarted( bool bTournamentModeOn ) {}
};

#endif // LOG_STATS_H
