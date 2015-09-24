
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

#include "mathlib/mathlib.h"

using namespace SCHelpers;

unsigned int CTFPlayerWrapper::flags_offset = 0;
unsigned int CTFPlayerWrapper::class_offset = 0;
unsigned int CTFPlayerWrapper::weapons_offset = 0;
unsigned int CTFPlayerWrapper::release_offset = 0;
unsigned int CTFPlayerWrapper::origin_offset = 0;
unsigned int CTFPlayerWrapper::charge_offset = 0;
unsigned int CTFPlayerWrapper::health_offset = 0;

CTFPlayerWrapper::CTFPlayerWrapper( CBaseEntity *pPlayer /*= nullptr*/ ):
	m_pPlayer(pPlayer)
{
}

void CTFPlayerWrapper::InitializeOffsets()
{
	flags_offset = GetPropOffsetFromTable("DT_BasePlayer", "m_fFlags");
	class_offset = GetPropOffsetFromTable("DT_TFPlayer", "m_PlayerClass") + 
		GetPropOffsetFromTable("DT_TFPlayerClassShared", "m_iClass");

	// should get the 0 offsets before it incase something changes
	weapons_offset = GetPropOffsetFromTable("DT_BaseCombatCharacter", "m_hMyWeapons");

	origin_offset = GetPropOffsetFromTable( "DT_BaseEntity", "m_vecOrigin" );
	release_offset = GetPropOffsetFromTable( "DT_WeaponMedigun", "m_bChargeRelease" );

	// should get the 0 offsets before it incase something changes
	charge_offset = GetPropOffsetFromTable("DT_LocalTFWeaponMedigunData", "m_flChargeLevel");

	health_offset = GetPropOffsetFromTable("DT_BasePlayer", "m_iHealth");
}

void CTFPlayerWrapper::SetPlayer( CBaseEntity *pPlayer )
{
	m_pPlayer = pPlayer;
}

unsigned int CTFPlayerWrapper::GetFlags() const
{
	unsigned int flags = 0;
	if (m_pPlayer)
	{
		flags = *ByteOffsetFromPointer<unsigned int*>(m_pPlayer, flags_offset);
	}
	return flags;
}

unsigned int CTFPlayerWrapper::GetClass() const
{
	unsigned int playerclass = 0;
	if (m_pPlayer)
	{
		playerclass = *ByteOffsetFromPointer<unsigned int*>(m_pPlayer, class_offset);
	}
	return playerclass;
}

int CTFPlayerWrapper::GetHealth() const
{
	int health = 0;
	if (m_pPlayer)
	{
		health = *ByteOffsetFromPointer<int*>(m_pPlayer, health_offset);
	}
	return health;
}

void CTFPlayerWrapper::SetHealth(int health)
{
	if (m_pPlayer)
	{
		*ByteOffsetFromPointer<int*>(m_pPlayer, health_offset) = health;
	}
}

CBaseHandle *CTFPlayerWrapper::GetWeapon( unsigned int slot ) const
{
	CBaseHandle *hWeapon = nullptr;
	if (m_pPlayer)
	{
		hWeapon = SCHelpers::ByteOffsetFromPointer<CBaseHandle*>(m_pPlayer, weapons_offset + 4*(slot-1));
	}
	return hWeapon;
}

float CTFPlayerWrapper::GetChargeLevel( CSizzPluginContext *pPluginContext ) const
{
	return AccessChargeLevel(pPluginContext, ACCESS_GET, 0.0f);
}

void CTFPlayerWrapper::SetChargeLevel( CSizzPluginContext *pPluginContext, float charge_level )
{
	AccessChargeLevel(pPluginContext, ACCESS_SET, clamp(charge_level, 0.0f, 1.0f));
}

Vector *CTFPlayerWrapper::GetPlayerOrigin() const
{
	Vector *pOrigin = nullptr;
	if (m_pPlayer)
	{
		pOrigin = ByteOffsetFromPointer<Vector*>(m_pPlayer, origin_offset);
	}
	return pOrigin;
}

bool CTFPlayerWrapper::IsReleasingCharge() const
{
	bool releasing = false;
	if (m_pPlayer)
	{
		releasing = *ByteOffsetFromPointer<bool*>(m_pPlayer, release_offset);
	}
	return releasing;
}

float CTFPlayerWrapper::AccessChargeLevel( CSizzPluginContext *pPluginContext, ACCESS_METHOD access, float set_charge ) const
{
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
				if (access == ACCESS_GET)
				{
					charge = *SCHelpers::ByteOffsetFromPointer<float*>(pMedigun, charge_offset);
				}
				else
				{
					*SCHelpers::ByteOffsetFromPointer<float*>(pMedigun, charge_offset) = set_charge;
				}
			}
		}
	}
	return charge;
}
