
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
	CTFPlayerWrapper();

	// setup
	void Initialize( CSizzPluginContext *pPluginContext, CBaseEntity *pPlayer = nullptr );

	// set the player that the data will come from
	void SetPlayer( CBaseEntity *pPlayer );

	// returns the m_fFlags for the player
	unsigned int GetFlags() const;

	// returns the player class
	unsigned int GetClass() const;

	// returns a handle to the weapon in the slot
	// slots start at 1 and go up
	CBaseHandle *GetWeapon( unsigned int slot ) const;

	// returns the charge level of the medic
	// returns 0.0f for non medics
	float GetChargeLevel( CSizzPluginContext *pPluginContext ) const;

	// returns true if the player is a medic 
	// and is currently releasing ubercharge
	bool IsReleasingCharge() const;

	// returns the player origin
	Vector *GetPlayerOrigin() const;

private:
	CBaseEntity *m_pPlayer;
};
