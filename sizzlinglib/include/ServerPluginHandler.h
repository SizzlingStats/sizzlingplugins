
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

// reversed from valve binaries

#ifndef SERVER_PLUGIN_HANDLER_H
#define SERVER_PLUGIN_HANDLER_H

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
	// some unreferenced member
	// not part of a CUtlVector update
	uint32 m_unk;
};

#endif // SERVER_PLUGIN_HANDLER_H
