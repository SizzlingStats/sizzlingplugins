
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/
#ifndef VFUNCS_H
#define VFUNCS_H

#ifdef WIN32
	#define DATA_DESC_OFFSET 11
	#define ACCEPT_INPUT_OFFSET 35
#else
	#define DATA_DESC_OFFSET 12
	#define ACCEPT_INPUT_OFFSET 36
#endif

#define GAME_DLL
#include "cbase.h"
#include "baseentity.h"
#include "variant_t.h"

class VfuncEmptyClass {};

static datamap_t *GetDataDescMap(CBaseEntity *pThisPtr)	// from sourcemod wiki virtual func examples
{
	void **this_ptr = *(void ***)&pThisPtr;
	void **vtable = *(void ***)pThisPtr;
	void *func = vtable[DATA_DESC_OFFSET];
 
	union {datamap_t *(VfuncEmptyClass::*mfpnew)();
#ifndef __linux__
        void *addr;	} u; 	u.addr = func;
#else /* GCC's member function pointers all contain a this pointer adjustor. You'd probably set it to 0 */
			struct {void *addr; intptr_t adjustor;} s; } u; u.s.addr = func; u.s.adjustor = 0;
#endif
 
	return (datamap_t *) (reinterpret_cast<VfuncEmptyClass*>(this_ptr)->*u.mfpnew)();
}

static datamap_t *GetDataDescMap(const CBaseEntity *pThisPtr)	// from sourcemod wiki virtual func examples
{
	void **this_ptr = *(void ***)&pThisPtr;
	void **vtable = *(void ***)pThisPtr;
	void *func = vtable[DATA_DESC_OFFSET];
 
	union {datamap_t *(VfuncEmptyClass::*mfpnew)();
#ifndef __linux__
        void *addr;	} u; 	u.addr = func;
#else /* GCC's member function pointers all contain a this pointer adjustor. You'd probably set it to 0 */
			struct {void *addr; intptr_t adjustor;} s; } u; u.s.addr = func; u.s.adjustor = 0;
#endif
 
	return (datamap_t *) (reinterpret_cast<VfuncEmptyClass*>(this_ptr)->*u.mfpnew)();
}

static bool AcceptInput( const char *szInputName, CBaseEntity *pActivator, CBaseEntity *pCaller, variant_t Value, int outputID )
{
	void **this_ptr = *(void ***)&szInputName;
	void **vtable = *(void ***)szInputName;
	void *func = vtable[ACCEPT_INPUT_OFFSET];
 
	union {bool (VfuncEmptyClass::*mfpnew)();
#ifndef __linux__
        void *addr;	} u; 	u.addr = func;
#else /* GCC's member function pointers all contain a this pointer adjustor. You'd probably set it to 0 */
			struct {void *addr; intptr_t adjustor;} s; } u; u.s.addr = func; u.s.adjustor = 0;
#endif
 
	return (bool) (reinterpret_cast<VfuncEmptyClass*>(this_ptr)->*u.mfpnew)();
}

#endif	// VFUNCS_H
