
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

#endif // SERVER_PLUGIN_HANDLER_H
