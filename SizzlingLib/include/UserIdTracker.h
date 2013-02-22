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

#endif // USER_ID_TRACKER_H
