
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "SizzPlugin.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Interfaces from the engine
IVEngineServer	*engine = NULL; // helper functions (messaging clients, loading content, making entities, running commands, etc)
IGameEventManager *gameeventmanager = NULL; // game events interface
IPlayerInfoManager *playerinfomanager = NULL; // game dll interface to interact with players
IBotManager *botmanager = NULL; // game dll interface to interact with bots
IServerPluginHelpers *helpers = NULL; // special 3rd party plugin helpers from the engine
IUniformRandomStream *randomStr = NULL;
IEngineTrace *enginetrace = NULL;

CGlobalVars *gpGlobals = NULL;

//---------------------------------------------------------------------------------
// Purpose: a sample 3rd party plugin class
//---------------------------------------------------------------------------------
class CEmptyServerPlugin: public CSizzPlugin
{
public:
	CEmptyServerPlugin();
	virtual ~CEmptyServerPlugin();

	virtual const char *GetPluginDescription( void );

	virtual bool	SizzLoad(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
	virtual void	SizzUnload();
	
	virtual void	SizzLevelInit( char const *pMapName );
	virtual void	SizzServerActivate( edict_t *pEdictList, int edictCount, int clientMax );
	virtual void	SizzGameFrame( bool simulating );
	virtual void	SizzLevelShutdown();

	virtual void	SizzClientConnect( edict_t *pEdict );
	virtual void	SizzClientDisconnect( edict_t *pEdict );

	// ISizzGameCallbacks
	virtual void	TournamentGameStarted();
	virtual void	TournamentRoundStarted();
	virtual void	TournamentRoundOver();
	virtual void	TournamentGameOver();
};

// 
// The plugin is a static singleton that is exported as an interface
//
CEmptyServerPlugin g_EmptyServerPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CEmptyServerPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS_VERSION_1, g_EmptyServerPlugin );

CEmptyServerPlugin::CEmptyServerPlugin(): CSizzPlugin()
{
}

CEmptyServerPlugin::~CEmptyServerPlugin()
{
}

const char *CEmptyServerPlugin::GetPluginDescription()
{
	return "";
}

bool CEmptyServerPlugin::SizzLoad(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
	return true;
}

void CEmptyServerPlugin::SizzUnload()
{
}

void CEmptyServerPlugin::SizzLevelInit(char const *pMapName)
{
}

void CEmptyServerPlugin::SizzServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
}

void CEmptyServerPlugin::SizzGameFrame( bool simulating )
{
}

void CEmptyServerPlugin::SizzLevelShutdown()
{
}

void CEmptyServerPlugin::SizzClientConnect( edict_t *pEdict )
{
}

void CEmptyServerPlugin::SizzClientDisconnect( edict_t *pEdict )
{
}

void CEmptyServerPlugin::TournamentGameStarted()
{
}

void CEmptyServerPlugin::TournamentRoundStarted()
{
}

void CEmptyServerPlugin::TournamentRoundOver()
{
}

void CEmptyServerPlugin::TournamentGameOver()
{
}