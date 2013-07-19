
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: playerdata.h
////////////////////////////////////////////////////////////////////////////////
#ifndef PLAYERDATA_H
#define PLAYERDATA_H

class CBaseEntity;
class IPlayerInfo;
struct edict_t;

////////////////////////////////////////////////////////////////////////////////
// Class name: BasePlayerData
////////////////////////////////////////////////////////////////////////////////
class BasePlayerData
{
public:
	BasePlayerData();
	BasePlayerData( edict_t *pEdict, IPlayerInfo *pInfo );

	void SetBaseData( edict_t *pEdict, IPlayerInfo *pInfo );
	virtual ~BasePlayerData();

	CBaseEntity *GetBaseEntity() const
	{
		return m_pPlayerEnt;
	}

	IPlayerInfo *GetPlayerInfo() const
	{
		return m_pPlayerInfo;
	}

	edict_t	*GetEdict() const
	{
		return m_pEdict;
	}

	int	GetEntIndex() const
	{
		return m_iEntIndex;
	}

private:
	CBaseEntity *m_pPlayerEnt;
	IPlayerInfo *m_pPlayerInfo;
	edict_t	*m_pEdict;
	int	m_iEntIndex;
};

////////////////////////////////////////////////////////////////////////////////
// Class name: SM_PlayerData
////////////////////////////////////////////////////////////////////////////////
class SM_PlayerData: public BasePlayerData
{
public:
	SM_PlayerData();
	SM_PlayerData( edict_t *pEdict, IPlayerInfo *pInfo );
	virtual ~SM_PlayerData();

	void		UpdateName( const char *szNewName );
	void		SetReadyState( bool state );
	void		SetTeam( int teamindex );

	int			GetTeam();
	const char *GetName();
	bool		GetReadyState();
	
private:
	char m_szName[32];
	int m_iTeam;
	bool m_bIsReady;
};

#endif //PLAYERDATA_H
