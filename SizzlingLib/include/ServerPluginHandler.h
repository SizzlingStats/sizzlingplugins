
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef SERVER_PLUGIN_HANDLER_H
#define SERVER_PLUGIN_HANDLER_H

// This structure was reverse engineered 
// and found to overlap IServerPluginHelpers.
// It was named 's_ServerPlugin' as well.

// so just do a cast like this:
// struct s_ServerPlugin *pServerPluginHandler = (s_ServerPlugin*)pServerPluginHelpers;

typedef struct s_ServerPlugin //size 28 i'm pretty sure
{
	char unk[16];

	// number of loaded plugins
	unsigned int nLoadedPlugins;

	// description strings of the loaded plugins
	const char ***pszInfo;
} s_ServerPlugin;

extern s_ServerPlugin	*g_pServerPluginHandler;

#include "utlvector.h"
#include "engine/iserverplugin.h"
#include "platform.h"

class CSysModule;

class CPlugin
{
public:
	char m_szName[128];
	bool m_bDisabled;
	char padding[3];
	IServerPluginCallbacks *m_pPlugin;
	int m_iInterfaceVersion;
	CSysModule *m_pSysModule;
};

class CServerPlugin: public IServerPluginHelpers
{
public:
	CUtlVector<CPlugin*> m_plugins;
	// this member might be part of
	// a new version of the vector
	uint32 m_unk;
};

#endif // SERVER_PLUGIN_HANDLER_H
