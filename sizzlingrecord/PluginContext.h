
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef PLUGIN_CONTEXT_H
#define PLUGIN_CONTEXT_H

class IEngineSound;
class IGameEventManager2;
class IBaseClientDLL;
class IVEngineClient;
class IFileSystem;
class CGlobalVars;

typedef struct PluginContext_s
{
	IEngineSound *m_pEngineSound;
	IGameEventManager2 *m_pGameEventManager;
	IBaseClientDLL *m_pBaseClientDLL;
	IVEngineClient *m_pEngineClient;
	IFileSystem	*m_pFullFileSystem;
	CGlobalVars	*m_pGlobals;
} PluginContext_t;

#endif // PLUGIN_CONTEXT_H