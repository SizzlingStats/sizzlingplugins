
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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IVEngineServer		*pEngine;
extern IPlayerInfoManager	*playerinfomanager;
extern CGlobalVars			*gpGlobals;

int MRecipientFilter::GetRecipientCount() const
{
	return m_Recipients.Size();
}

int MRecipientFilter::GetRecipientIndex(int slot) const
{
	if(slot < 0 || slot >= GetRecipientCount())
		return -1;

	return m_Recipients[slot];
}

void MRecipientFilter::AddAllPlayers()
{
	m_Recipients.RemoveAll();

	for(int i = 1; i <= gpGlobals->maxClients; i++)
	{
		edict_t *pPlayer = pEngine->PEntityOfEntIndex(i);

		if(!pPlayer || pPlayer->IsFree())
			continue;

		IPlayerInfo *pInfo = playerinfomanager->GetPlayerInfo( pPlayer );
		if (pInfo)
			if ( !SCHelpers::FStrEq( pInfo->GetNetworkIDString(), "BOT" ) )
				m_Recipients.AddToTail(i);
	}
} 
void MRecipientFilter::AddRecipient(int iPlayer)
{
	// Return if the recipient is already in the vector
	if(m_Recipients.Find(iPlayer) != m_Recipients.InvalidIndex())
		return;

	// Make sure the player is valid
	edict_t* pPlayer = pEngine->PEntityOfEntIndex(iPlayer);
	if(!pPlayer || pPlayer->IsFree())
		return;

	m_Recipients.AddToTail(iPlayer);
}

#include "SizzPluginContext.h"

MRecipientFilter_new::MRecipientFilter_new( CSizzPluginContext &context ):
	m_context(&context)
{
}

int MRecipientFilter_new::GetRecipientCount( void ) const
{
	return m_recipients.Count();
}

int MRecipientFilter_new::GetRecipientIndex( int slot ) const
{
	if (slot < 0 || slot >= GetRecipientCount())
	{
		return -1;
	}

	return m_recipients[slot];
}

void MRecipientFilter_new::AddAllPlayers()
{
	m_recipients.RemoveAll();
	int max_clients = m_context->GetMaxClients();
	for (int i = 1; i <= max_clients; ++i)
	{
		IPlayerInfo *pInfo = m_context->GetPlayerInfo(i);
		if (pInfo && !pInfo->IsFakeClient())
		{
			m_recipients.AddToTail(i);
		}
	}
}

void MRecipientFilter_new::AddRecipient(int ent_index)
{
	// Return if the recipient is already in the vector
	if (m_recipients.Find(ent_index) == m_recipients.InvalidIndex())
	{
		IPlayerInfo *pInfo = m_context->GetPlayerInfo(ent_index);
		if (pInfo && !pInfo->IsFakeClient())
		{
			m_recipients.AddToTail(ent_index);
		}
	}
}
