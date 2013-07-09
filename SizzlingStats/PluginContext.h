
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

class IVEngineServer;
class IGameEventManager2;
class IPlayerInfoManager;
class CServerPlugin;
class IServerPluginHelpers;

class CPluginContext
{
	friend class CEmptyServerPlugin;
public:
	IVEngineServer *GetEngine() const;
	IGameEventManager2 *GetEventMgr() const;
	IPlayerInfoManager *GetPlayerInfoMgr() const;
	CServerPlugin *GetPluginManager() const;
	IServerPluginHelpers *GetServerPluginHelpers() const;

protected:
	IVEngineServer *m_pEngineServer;
	IGameEventManager2 *m_pGameEventManager;
	IPlayerInfoManager *m_pPlayerInfoManager;
	union
	{
		CServerPlugin *m_pPluginManager;
		IServerPluginHelpers *m_pPluginHelpers;
	};
};

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

inline CServerPlugin *CPluginContext::GetPluginManager() const
{
	return m_pPluginManager;
}

inline IServerPluginHelpers *CPluginContext::GetServerPluginHelpers() const
{
	return m_pPluginHelpers;
}

#endif // PLUGIN_CONTEXT_H
