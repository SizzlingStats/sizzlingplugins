
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#pragma once

#include "mathlib/vector.h"

class CSizzPluginContext;
class CBaseEntity;
class CBaseHandle;

class CTFPlayerWrapper
{
public:
	CTFPlayerWrapper( CBaseEntity *pPlayer = nullptr );

	// setup
	static void InitializeOffsets();

	// set the player that the data will come from
	void SetPlayer( CBaseEntity *pPlayer );

	// returns the m_fFlags for the player
	unsigned int GetFlags() const;

	// returns the player class
	unsigned int GetClass() const;

	int GetHealth() const;
	void SetHealth(int health);

	// returns a handle to the weapon in the slot
	// slots start at 1 and go up
	CBaseHandle *GetWeapon( unsigned int slot ) const;

	// returns the charge level of the medic
	// returns 0.0f for non medics
	float GetChargeLevel( CSizzPluginContext *pPluginContext ) const;
	void SetChargeLevel( CSizzPluginContext *pPluginContext, float charge_level );

	// returns true if the player is a medic 
	// and is currently releasing ubercharge
	bool IsReleasingCharge() const;

	// returns the player origin
	Vector *GetPlayerOrigin() const;

private:
	static unsigned int flags_offset;
	static unsigned int class_offset;
	static unsigned int weapons_offset;
	static unsigned int release_offset;
	static unsigned int origin_offset;
	static unsigned int charge_offset;
	static unsigned int health_offset;

private:
	enum ACCESS_METHOD
	{
		ACCESS_GET = 0,
		ACCESS_SET = 1
	};
	float AccessChargeLevel( CSizzPluginContext *pPluginContext, ACCESS_METHOD access, float set_charge ) const;

private:
	CBaseEntity *m_pPlayer;
};
