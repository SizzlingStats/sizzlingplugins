
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef STEAM_GAME_SERVER_H
#define STEAM_GAME_SERVER_H

#define NO_CSTEAMID_STL
#include "open_steamworks/Steamworks.h"

#include "interface.h"

typedef HSteamPipe (*GetSteamPipeFn)();
typedef HSteamUser (*GetSteamUserFn)();

static ISteamGameServer011 *GetSteamServerInterface()
{
	ISteamGameServer011 *pSteamGameServer = NULL;
	CSysModule *hSteamApi = Sys_LoadModule( "steam_api" );
	if (hSteamApi)
	{
		GetSteamPipeFn fnGameServerSteamPipe = (GetSteamPipeFn)GetProcAddress( (HMODULE)hSteamApi, "SteamGameServer_GetHSteamPipe" );
		GetSteamUserFn fnGameServerSteamUser = (GetSteamPipeFn)GetProcAddress( (HMODULE)hSteamApi, "SteamGameServer_GetHSteamUser" );

#ifdef _WIN32
		HMODULE hSteamClient = LoadLibrary("steamclient");
#else
		HMODULE hSteamClient = dlopen("steamclient", RTLD_NOW);
#endif
		if (hSteamClient)
		{
			CreateInterfaceFn steamFactory = (CreateInterfaceFn)GetProcAddress( (HMODULE)hSteamClient, "CreateInterface" );
			if (steamFactory && fnGameServerSteamPipe && fnGameServerSteamUser)
			{
				ISteamClient012 *pSteamClient = (ISteamClient012*)steamFactory(STEAMCLIENT_INTERFACE_VERSION_012, NULL);
				if (pSteamClient)
				{
					HSteamPipe pipe = fnGameServerSteamPipe();
					HSteamUser user = fnGameServerSteamUser();

					pSteamGameServer = (ISteamGameServer011*)pSteamClient->GetISteamGameServer(user, pipe, STEAMGAMESERVER_INTERFACE_VERSION_011);
				}
			}
			Sys_UnloadModule((CSysModule*)hSteamClient);
		}
		Sys_UnloadModule(hSteamApi);
	}
	return pSteamGameServer;
}

#endif // STEAM_GAME_SERVER_H
