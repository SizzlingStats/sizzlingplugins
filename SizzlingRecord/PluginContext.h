
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