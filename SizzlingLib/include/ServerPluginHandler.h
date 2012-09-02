// ---------------------
//
//
// ---------------------

#ifndef SERVER_PLUGIN_HANDLER_H
#define SERVER_PLUGIN_HANDLER_H

struct s_ServerPlugin //size 28 i'm pretty sure
{
	char unk[16];

	// number of loaded plugins
	unsigned int nLoadedPlugins;

	// description strings of the loaded plugins
	const char ***pszInfo;
};

extern struct s_ServerPlugin	*g_pServerPluginHandler;

#endif // SERVER_PLUGIN_HANDLER_H