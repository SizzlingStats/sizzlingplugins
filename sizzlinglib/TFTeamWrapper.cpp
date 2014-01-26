
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "TFTeamWrapper.h"

#include "SC_helpers.h"

using namespace SCHelpers;

unsigned int CTFTeamWrapper::teamid_offset = 0;
unsigned int CTFTeamWrapper::score_offset = 0;

CTFTeamWrapper::CTFTeamWrapper( CBaseEntity *pTFTeam /*= nullptr*/ ):
	m_pTeam(pTFTeam)
{
}

void CTFTeamWrapper::InitializeOffsets()
{
	teamid_offset = GetPropOffsetFromTable( "DT_Team", "m_iTeamNum" );
	score_offset = GetPropOffsetFromTable( "DT_Team", "m_iScore" );
}

void CTFTeamWrapper::SetTeam( CBaseEntity *pTFTeam )
{
	m_pTeam = pTFTeam;
}

unsigned int CTFTeamWrapper::GetTeamID() const
{
	unsigned int teamid = 0;
	if (m_pTeam)
	{
		teamid = *ByteOffsetFromPointer<unsigned int*>(m_pTeam, teamid_offset);
	}
	return teamid;
}

unsigned int CTFTeamWrapper::GetScore() const
{
	unsigned int score = 0;
	if (m_pTeam)
	{
		score = *SCHelpers::ByteOffsetFromPointer<unsigned int*>(m_pTeam, score_offset);
	}
	return score;
}
