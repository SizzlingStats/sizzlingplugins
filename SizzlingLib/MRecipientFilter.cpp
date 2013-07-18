
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

//----------------------------------------

#include "MRecipientFilter.h"
#include "engine/iserverplugin.h"
#include "game/server/iplayerinfo.h"
#include "eiface.h"
#include "SC_helpers.h"

#include "SizzPluginContext.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void MRecipientFilter::AddAllPlayers( CSizzPluginContext *context )
{
	Assert(context);
	if (context)
	{
		m_recipients.RemoveAll();
		int max_clients = context->GetMaxClients();
		for (int i = 1; i <= max_clients; ++i)
		{
			IPlayerInfo *pInfo = context->GetPlayerInfo(i);
			if (pInfo && !pInfo->IsFakeClient())
			{
				m_recipients.AddToTail(i);
			}
		}
	}
}