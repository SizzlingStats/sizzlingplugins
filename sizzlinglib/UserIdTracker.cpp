
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
#include "strtools.h"

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
