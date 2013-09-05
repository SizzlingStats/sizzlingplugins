
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#pragma once

class CBaseEntity;

class CTFTeamWrapper
{
public:
	CTFTeamWrapper( CBaseEntity *pTFTeam = nullptr );

	// initialize the data offsets
	static void InitializeOffsets();

	// sets the team entity
	void SetTeam( CBaseEntity *pTFTeam );

	// returns the id of the team
	unsigned int GetTeamID() const;

	// returns the current score of the team
	unsigned int GetScore() const;

private:
	static unsigned int teamid_offset;
	static unsigned int score_offset;

private:
	CBaseEntity *m_pTeam;
};
