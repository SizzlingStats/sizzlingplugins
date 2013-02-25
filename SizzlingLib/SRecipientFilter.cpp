
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

void SRecipientFilter::AddRecipient(int iPlayer)
{
	// Make sure the player is valid
	edict_t* pPlayer = pEngine->PEntityOfEntIndex(iPlayer);
	if(!pPlayer || pPlayer->IsFree())
		return;

	m_Recipient = iPlayer;
}
