////////////////////////////////////////////////////////////////////////////////
// Filename: PlayerMessage.h
////////////////////////////////////////////////////////////////////////////////
#ifndef PLAYER_MESSAGE_H
#define PLAYER_MESSAGE_H

struct edict_t;

struct colour
{
	char r, g, b, a;
	colour(): r(0), g(0), b(0), a(0)
	{
	}
};

class PlayerMessage
{
public:
	PlayerMessage(void);
	~PlayerMessage(void);

	void		SingleUserChatMessage( edict_t *pEntity, const char *szMessage );
	void		SingleUserChatMessage( int entindex, const char *szMessage );
	void		SingleUserChatMessage( edict_t *pEntity, const char *szMessage, const char *szPrefix );
	void		SingleUserChatMessage( int entindex, const char *szMessage, const char *szPrefix );
	void		AllUserChatMessage( const char *szMessage );
	void		AllUserChatMessage( const char *szMessage, const char *szPrefix );
	void		AllUserHudReset();

	void		AllUserHudMsg( const char *szMessage, colour rgba, float timeonscreen, float x = -1, float y = -1, int channel = 1 );
	void		AllUserHudHintText( const char *szMessage );

	void		SingleUserVGUIMenu( int clientIndex, const char *title, const char *url );
	void		SingleUserEmptyVGUIMenu( int clientIndex );

private:

};

extern PlayerMessage *g_pMessage;

#endif // PLAYER_MESSAGE_H
