
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: SRecipientFilter.cpp
////////////////////////////////////////////////////////////////////////////////

#include "SRecipientFilter.h"
#include "eiface.h"

extern IVEngineServer		*pEngine;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

int SRecipientFilter::GetRecipientIndex(int slot) const
{
	if(slot < 0 || slot >= GetRecipientCount())
		return -1;

	return m_Recipient;
}

void SRecipientFilter::SetRecipient(int iPlayer)
{
	// Make sure the player is valid
	edict_t* pPlayer = pEngine->PEntityOfEntIndex(iPlayer);
	if(!pPlayer || pPlayer->IsFree())
		return;

	m_Recipient = iPlayer;
}

#include "SizzPluginContext.h"
#include "game/server/iplayerinfo.h"

SRecipientFilter_new::SRecipientFilter_new( CSizzPluginContext &context, int ent_index /*= -1*/ ):
	m_context(&context),
	m_recipient(-1)
{
	SetRecipient(ent_index);
}

int SRecipientFilter_new::GetRecipientIndex( int slot ) const
{
	if (slot != 0)
	{
		return -1;
	}
	else
	{
		return m_recipient;
	}
}

void SRecipientFilter_new::SetRecipient( int ent_index )
{
	// Make sure the player is valid
	IPlayerInfo *pInfo = m_context->GetPlayerInfo(ent_index);
	if (pInfo && !pInfo->IsFakeClient())
	{
		m_recipient = ent_index;
	}
}
