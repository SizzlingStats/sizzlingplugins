
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#pragma once

#ifndef PLUGIN_CONTEXT_H
#define PLUGIN_CONTEXT_H

#include "igameevents.h"
#include "eiface.h"
#include "game/server/iplayerinfo.h"

class CPluginContext
{
	friend class CEmptyServerPlugin;
public:
	CPluginContext();

	IVEngineServer *GetEngine() const;
	IGameEventManager2 *GetEventMgr() const;
	IPlayerInfoManager *GetPlayerInfoMgr() const;

protected:
	void SetInterfaces(
		IVEngineServer *pEng = NULL, 
		IGameEventManager2 *pEventMgr = NULL,
		IPlayerInfoManager *pPlayerInfoMgr = NULL );

private:
	IVEngineServer *m_pEngineServer;
	IGameEventManager2 *m_pGameEventManager;
	IPlayerInfoManager *m_pPlayerInfoManager;
};

inline CPluginContext::CPluginContext():
	m_pEngineServer(NULL),
	m_pGameEventManager(NULL),
	m_pPlayerInfoManager(NULL)
{
}

inline IVEngineServer *CPluginContext::GetEngine() const
{
	return m_pEngineServer;
}

inline IGameEventManager2 *CPluginContext::GetEventMgr() const
{
	return m_pGameEventManager;
}

inline IPlayerInfoManager *CPluginContext::GetPlayerInfoMgr() const
{
	return m_pPlayerInfoManager;
}

inline void CPluginContext::SetInterfaces(
		IVEngineServer *pEng /*= NULL*/, 
		IGameEventManager2 *pEventMgr /*= NULL*/,
		IPlayerInfoManager *pPlayerInfoMgr /*= NULL*/  )
{
	m_pEngineServer = pEng;
	m_pGameEventManager = pEventMgr;
	m_pPlayerInfoManager = pPlayerInfoMgr;
}

#endif // PLUGIN_CONTEXT_H
