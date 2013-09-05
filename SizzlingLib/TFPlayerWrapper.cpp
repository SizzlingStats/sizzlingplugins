
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "TFPlayerWrapper.h"

#include "SC_helpers.h"
#include "SizzPluginContext.h"

using namespace SCHelpers;

CTFPlayerWrapper::CTFPlayerWrapper():
	m_pPlayer(nullptr)
{
}

void CTFPlayerWrapper::Initialize( CSizzPluginContext *pPluginContext, CBaseEntity *pPlayer /*= nullptr*/ )
{
	SetPlayer(pPlayer);
	// call each method once to init the static vars
	// for performance reasons
	//
	// should make sure that nothing bad happens
	// when calling these functions on worldspawn
	GetFlags();
	GetClass();
	GetWeapon(0);
	GetChargeLevel(pPluginContext);
	GetPlayerOrigin();
	IsReleasingCharge();
}

void CTFPlayerWrapper::SetPlayer( CBaseEntity *pPlayer )
{
	m_pPlayer = pPlayer;
}

unsigned int CTFPlayerWrapper::GetFlags() const
{
	static unsigned int flags_offset = GetPropOffsetFromTable("DT_BasePlayer", "m_fFlags");

	unsigned int flags = 0;
	if (m_pPlayer)
	{
		flags = *ByteOffsetFromPointer<unsigned int*>(m_pPlayer, flags_offset);
	}
	return flags;
}

unsigned int CTFPlayerWrapper::GetClass() const
{
	static unsigned int class_offset = GetPropOffsetFromTable("DT_TFPlayer", "m_PlayerClass") + 
		GetPropOffsetFromTable("DT_TFPlayerClassShared", "m_iClass");

	unsigned int playerclass = 0;
	if (m_pPlayer)
	{
		playerclass = *ByteOffsetFromPointer<unsigned int*>(m_pPlayer, class_offset);
	}
	return playerclass;
}

CBaseHandle *CTFPlayerWrapper::GetWeapon( unsigned int slot ) const
{
	// should get the 0 offsets before it incase something changes
	static unsigned int weapons_offset = GetPropOffsetFromTable("DT_BaseCombatCharacter", "m_hMyWeapons");

	CBaseHandle *hWeapon = nullptr;
	if (m_pPlayer)
	{
		hWeapon = SCHelpers::ByteOffsetFromPointer<CBaseHandle*>(m_pPlayer, weapons_offset + 4*(slot-1));
	}
	return hWeapon;
}

float CTFPlayerWrapper::GetChargeLevel( CSizzPluginContext *pPluginContext ) const
{
	// should get the 0 offsets before it incase something changes
	static unsigned int charge_offset = GetPropOffsetFromTable("DT_LocalTFWeaponMedigunData", "m_flChargeLevel");

	float charge = 0.0f;
	if (m_pPlayer)
	{
		// if the player is a medic
		// TODO: change 5 to an enum of classes or something
		if (GetClass() == 5)
		{
			// get the medigun; slot 2
			CBaseHandle *hMedigun = GetWeapon(2);
			CBaseEntity *pMedigun = pPluginContext->BaseEntityFromBaseHandle(hMedigun);
			if (pMedigun)
			{
				charge = *SCHelpers::ByteOffsetFromPointer<float*>(pMedigun, charge_offset);
			}
		}
	}
	return charge;
}

Vector *CTFPlayerWrapper::GetPlayerOrigin() const
{
	static unsigned int origin_offset = GetPropOffsetFromTable( "DT_BaseEntity", "m_vecOrigin" );

	Vector *pOrigin = nullptr;
	if (m_pPlayer)
	{
		pOrigin = ByteOffsetFromPointer<Vector*>(m_pPlayer, origin_offset);
	}
	return pOrigin;
}

bool CTFPlayerWrapper::IsReleasingCharge() const
{
	static unsigned int release_offset = GetPropOffsetFromTable( "DT_WeaponMedigun", "m_bChargeRelease" );

	bool releasing = false;
	if (m_pPlayer)
	{
		releasing = *ByteOffsetFromPointer<bool*>(m_pPlayer, release_offset);
	}
	return releasing;
}
