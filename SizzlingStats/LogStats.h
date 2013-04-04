
#pragma once

#ifndef LOG_STATS_H
#define LOG_STATS_H

#include "platform.h"
#include "igameevents.h"

class IGameEventManager2;
class IVEngineServer;
struct edict_t;
class IGameEvent;
class CPluginContext;
struct playerInfo;

class CLogStats: public IGameEventListener2
{
public:
	CLogStats( const CPluginContext &plugin_context );
	~CLogStats();

	bool Load();
	void Unload();

	void LevelInit( const char *pMapName );

	void ClientActive( edict_t *pEdict, int ent_index );
	void ClientDisconnect( edict_t *pEdict );

	void TournamentMatchStarted();
	void TournamentMatchEnded();

	virtual void FireGameEvent( IGameEvent *event );

private:
	const CPluginContext &m_context;
	playerInfo *m_entIndexToPlayerInfo;
};

#endif // LOG_STATS_H
