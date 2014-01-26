
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef SIZZ_PLUGIN_H
#define SIZZ_PLUGIN_H

#include <stdio.h>

#include "interface.h"
#include "filesystem.h"
#include "engine/iserverplugin.h"
#include "game/server/iplayerinfo.h"
#include "eiface.h"
#include "igameevents.h"
#include "convar.h"
#include "Color.h"
#include "vstdlib/random.h"
#include "engine/IEngineTrace.h"
#include "tier2/tier2.h"

#include "UserIdTracker.h"

abstract_class ISizzGameCallbacks
{
public:
	virtual void	TournamentGameStarted() = 0;
	virtual void	TournamentRoundStarted() = 0;
	virtual void	TournamentRoundOver() = 0;
	virtual void	TournamentGameOver() = 0;
};

abstract_class ISizzPluginCallbacks
{
public:
	virtual bool	SizzLoad(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) = 0;
	virtual void	SizzUnload() = 0;
	
	virtual void	SizzLevelInit( char const *pMapName ) = 0;
	virtual void	SizzServerActivate( edict_t *pEdictList, int edictCount, int clientMax ) = 0;
	virtual void	SizzGameFrame( bool simulating ) = 0;
	virtual void	SizzLevelShutdown() = 0;

	virtual void	SizzClientConnect( edict_t *pEdict ) = 0;
	virtual void	SizzClientDisconnect( edict_t *pEdict ) = 0;
};

//---------------------------------------------------------------------------------
// Purpose: a sample 3rd party plugin class
//---------------------------------------------------------------------------------
abstract_class CSizzPlugin: public IServerPluginCallbacks, public ISizzGameCallbacks, public ISizzPluginCallbacks
{
public:
	CSizzPlugin():
		m_iClientCommandIndex(0),
		m_nConnectedPlayers(0)
	{
	}

	virtual ~CSizzPlugin()
	{
	}

	// IServerPluginCallbacks methods
	virtual bool Load( CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory )
	{
		return SizzLoad(interfaceFactory, gameServerFactory);
	}

	virtual void Unload( void )
	{
		SizzUnload();
	}

	virtual void Pause( void )
	{
	}

	virtual void UnPause( void )
	{
	}

	virtual const char *GetPluginDescription( void ) = 0;

	virtual void LevelInit( char const *pMapName )
	{
		SizzLevelInit(pMapName);
	}

	virtual void ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
	{
		SizzServerActivate(pEdictList, edictCount, clientMax);
	}

	virtual void GameFrame( bool simulating )
	{
		SizzGameFrame(simulating);
	}

	virtual void LevelShutdown( void )
	{
		SizzLevelShutdown();
	}

private:
	virtual void ClientActive( edict_t *pEdict )
	{
		if (!pEdict || pEdict->IsFree())
		{
			return;
		}
		++m_nConnectedPlayers;
		SizzClientConnect(pEdict);
	}

	virtual void ClientDisconnect( edict_t *pEdict )
	{
		if (!pEdict || pEdict->IsFree())
		{
			return;
		}
		--m_nConnectedPlayers;
		SizzClientDisconnect(pEdict);
	}

public:
	virtual void ClientPutInServer( edict_t *pEntity, char const *playername )
	{
	}
	
	virtual void SetCommandClient( int index )
	{
		m_iClientCommandIndex = ++index;
	}

	virtual void ClientSettingsChanged( edict_t *pEdict )
	{
	}

	virtual PLUGIN_RESULT ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen )
	{
		return PLUGIN_CONTINUE;
	}

	virtual PLUGIN_RESULT ClientCommand( edict_t *pEntity, const CCommand &args )
	{
		return PLUGIN_CONTINUE;
	}

	virtual PLUGIN_RESULT NetworkIDValidated( const char *pszUserName, const char *pszNetworkID )
	{
		return PLUGIN_CONTINUE;
	}

	virtual void OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue )
	{
	}

	virtual void OnEdictAllocated( edict_t *edict )
	{
	}

	virtual void OnEdictFreed( const edict_t *edict )
	{
	}

	virtual int GetCommandIndex()
	{
		return m_iClientCommandIndex;
	}

	// Other helper functions
	unsigned int GetNumConnectedPlayers() const
	{
		return m_nConnectedPlayers;
	}

private:
	int m_iClientCommandIndex;
	unsigned int m_nConnectedPlayers;
};

#endif // SIZZ_PLUGIN_H