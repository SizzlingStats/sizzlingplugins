
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: UserIdTracker.h
////////////////////////////////////////////////////////////////////////////////
#ifndef USER_ID_TRACKER_H
#define USER_ID_TRACKER_H

struct edict_t;

class UserIdTracker
{
public:
	UserIdTracker(void);
	~UserIdTracker(void);

	void		Load();
	void		Reset();
	int 		ClientActive( edict_t *pEdict );
	void 		ClientDisconnect( edict_t *pEdict );
	int 		GetEntIndex( int userid ) const;

private:
	char m_entIndexTable[65536];
};

extern UserIdTracker *g_pUserIdTracker;

class CUserIDTracker
{
	friend class CSizzPluginContext;
public:
	int GetEntIndex( int userid ) const;

protected:
	void Reset();
	void ClientActive( int userid, int ent_index );
	void ClientDisconnect( int userid );

private:
	char m_ent_index_table[65536];
};

#endif // USER_ID_TRACKER_H
