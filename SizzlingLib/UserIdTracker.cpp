
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: UserIdTracker.cpp
////////////////////////////////////////////////////////////////////////////////
#include "UserIdTracker.h"

#include "eiface.h"

// Interfaces from the engine
extern IVEngineServer			*pEngine;

static UserIdTracker gUserIdTracker;
UserIdTracker *g_pUserIdTracker = &gUserIdTracker;

UserIdTracker::UserIdTracker()
{
	// increases binary size by the array size for some reason
	//Load();
}

UserIdTracker::~UserIdTracker()
{	
}

void UserIdTracker::Load()
{
	Reset();
}

void UserIdTracker::Reset()
{
	memset( m_entIndexTable, -1, 65536 );
}

int UserIdTracker::ClientActive( edict_t *pEdict )
{
	int userid = pEngine->GetPlayerUserId( pEdict );
	Assert(userid != -1);
	m_entIndexTable[userid] = pEngine->IndexOfEdict( pEdict );
	return static_cast<int>( m_entIndexTable[userid] );
}

void UserIdTracker::ClientDisconnect( edict_t *pEdict )
{
	int userid = pEngine->GetPlayerUserId( pEdict );
	m_entIndexTable[userid] = -1;
}

int UserIdTracker::GetEntIndex( int userid ) const
{
	return static_cast<int>( m_entIndexTable[userid] );
}

int CUserIDTracker::GetEntIndex( int userid ) const
{
	if (0 <= userid && userid < 65536)
	{
		return m_ent_index_table[userid];
	}
	return -1;
}

void CUserIDTracker::Reset()
{
	memset( m_ent_index_table, -1, 65536 );
}

void CUserIDTracker::ClientActive( int userid, int ent_index )
{
	m_ent_index_table[userid] = ent_index;
}

void CUserIDTracker::ClientDisconnect( int userid )
{
	m_ent_index_table[userid] = -1;
}
