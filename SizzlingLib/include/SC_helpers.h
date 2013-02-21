#ifndef SC_HELPERS_H
#define SC_HELPERS_H

#include "strtools.h"

#ifndef NULL
#define NULL 0
#endif

typedef unsigned char byte;

class CBaseEntity;
struct edict_t;
class SendTable;
class SendProp;
class CTeamplayRoundBasedRules;
struct datamap_t;
struct typedescription_t;
class CBaseHandle;

namespace SCHelpers
{
	inline bool FStrCmp( const char * const &pStr1, const char * const &pStr2 )
	{
		return ( V_strcmp( pStr1, pStr2 ) < 0 ) ? true : false;
	}

	inline bool FUIntCmp( const unsigned int &num1, const unsigned int &num2 )
	{
		return ( num1 < num2 );
	}

	inline bool FIntCmp( const int &num1, const int &num2 )
	{
		return ( num1 < num2 );
	}

	inline bool FStrEq( const char *sz1, const char *sz2 )
	{
		return(Q_stricmp(sz1, sz2) == 0);
	}
	
	inline uint64 RoundDBL( double num )
	{
		return static_cast<uint64>(num + 0.5);
	}

	template<typename T, typename U>
	inline T *ByteOffsetFromPointer( U *pBase, uint32 byte_offset )
	{
		return reinterpret_cast<T*>((reinterpret_cast<uint8*>(pBase) + byte_offset));
	}

	CBaseEntity *BaseHandleToBaseEntity( const CBaseHandle *pHandle );

	// gets the m_iClassname value for the entity
	// doesn't check pEnt for NULL
	const char **GetClassname( CBaseEntity * const pEnt );

	edict_t *UserIDToEdict( int userid );
	
	unsigned int UserIDToSteamID( int userid );

	CBaseEntity *GetEntityByClassname( const char *pszClassname, int start_index = 0, int *ent_index_out = NULL );

	void GetTeamEnts( CBaseEntity **ppBluTeam, CBaseEntity **ppRedTeam, uint32 team_num_offset );

	int UserIDToEntIndex( int userid );

	//-----------------------------------------------------------------------------
	// Purpose: Returns the 4 bit nibble for a hex character
	// Input  : c - 
	// Output : unsigned char
	//-----------------------------------------------------------------------------
	unsigned char S_nibble( char c );

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Input  : *in - 
	//			numchars - 
	//			*out - 
	//			maxoutputbytes - 
	//-----------------------------------------------------------------------------
	void S_bigendianhextobinary( char const *in, int numchars, byte *out, int maxoutputbytes );

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Input  : *in - 
	//			numchars - 
	//			*out - 
	//			maxoutputbytes - 
	//-----------------------------------------------------------------------------
	void S_littleendianhextobinary( char const *in, int numchars, byte *out, int maxoutputbytes );

	unsigned int GetThisPluginIndex( const char *pszDescriptionPart );

	//---------------------------------------------------------------------------------
	// Purpose: used by the GetPropOffsetFromTable func to get a specific table
	//---------------------------------------------------------------------------------
	SendTable *GetDataTable( const char *pTableName, SendTable *pTable );

	//---------------------------------------------------------------------------------
	// Purpose: returns the specified prop from the class and table provided.
	//			if prop or table not found, pointer returns NULL
	//---------------------------------------------------------------------------------
	SendProp *GetPropFromClassAndTable(const char *szClassName, const char *szTableName, const char *szPropName);
	
	//---------------------------------------------------------------------------------
	// Purpose: returns the specified prop offset relative to the table provided.
	//			if offset or table not found, bErr returns true and offset returned is 0
	//---------------------------------------------------------------------------------
	unsigned int GetPropOffsetFromTable(const char *pTableName, const char *pPropName, bool &bErr); //TODO: make this optional bool

	CTeamplayRoundBasedRules *GetTeamplayRoundBasedGameRulesPointer();

	int GetDatamapVarOffset( datamap_t *pDatamap, const char *szVarName );

	int GetOffsetForDatamapVar( const CBaseEntity *pEntity, const char *szVarName );

} // namespace SCHelpers

#endif	// SC_HELPERS_H
