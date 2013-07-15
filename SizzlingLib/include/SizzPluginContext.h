
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#pragma once

#ifndef SIZZ_PLUGIN_CONTEXT_H
#define SIZZ_PLUGIN_CONTEXT_H

class IVEngineServer;
class CGlobalVars;
class CUserIDTracker;
struct edict_t;

typedef struct plugin_context_init_s
{
	IVEngineServer *pEngine;
	CGlobalVars *pGlobals;
} plugin_context_init_t;

class CSizzPluginContext
{
	friend class CEmptyServerPlugin;
public:
	CSizzPluginContext();
	~CSizzPluginContext();

	void Initialize( const plugin_context_init_t &init );

	// returns -1 if ent index is invalid
	int UserIDFromEntIndex( int ent_index );

	// returns -1 if userid is invalid
	int EntIndexFromUserID( int userid );

	// returns -1 on error
	int SteamIDFromUserID( int userid );

	// returns -1 on error
	int SteamIDFromEntIndex( int ent_index );

	// the current server tick
	int GetCurrentTick() const;

	// the current time (per frame incremented)
	float GetTime() const;

protected:
	void ClientActive( const edict_t *pEdict );
	void ClientDisconnect( const edict_t *pEdict );
	void GameFrame( bool simulating );

private:
	IVEngineServer *m_pEngine;
	CGlobalVars *m_pGlobals;
	CUserIDTracker *m_pUserIDTracker;

	int m_tickcount;
	float m_flTime;
};

#endif // SIZZ_PLUGIN_CONTEXT_H
