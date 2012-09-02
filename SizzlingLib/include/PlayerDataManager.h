////////////////////////////////////////////////////////////////////////////////
// Filename: PlayerDataManager.h
////////////////////////////////////////////////////////////////////////////////
#ifndef PLAYER_DATA_MANAGER_H
#define PLAYER_DATA_MANAGER_H

//////////////
// INCLUDES //
//////////////
#include <stdio.h>

#include "engine/iserverplugin.h"
#include "game/server/iplayerinfo.h"
#include "eiface.h"
#include "igameevents.h"
#include "vstdlib/random.h"
#include "engine/IEngineTrace.h"

#include "tier1/utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Interfaces from the engine
extern IVEngineServer			*engine; // helper functions (messaging clients, loading content, making entities, running commands, etc)
extern IGameEventManager2		*gameeventmanager; // game events interface
extern IPlayerInfoManager		*playerinfomanager; // game dll interface to interact with players

extern IServerPluginHelpers		*helpers; // special 3rd party plugin helpers from the engine
extern IUniformRandomStream		*randomStr;
extern IEngineTrace				*enginetrace;

extern IServerGameDLL			*pServerDLL;
extern IServerGameEnts			*pServerEnts;
extern CGlobalVars				*gpGlobals;

class CBaseEntity;
class IPlayerInfo;
struct edict_t;

////////////////////////////////////////////////////////////////////////////////
// Class name: BasePlayerData
////////////////////////////////////////////////////////////////////////////////
class PlayerDataManager
{
public:
	PlayerDataManager();
	virtual ~PlayerDataManager();

private:
	CUtlVector<int> m_iEntIndex;
	CUtlVector<int> m_iUserID;
	CUtlVector<SS_PlayerData*> m_PlayerData;
};

PlayerDataManager::PlayerDataManager()
{
}

PlayerDataManager::~PlayerDataManager()
{
}



#endif //PLAYER_DATA_MANAGER_H
