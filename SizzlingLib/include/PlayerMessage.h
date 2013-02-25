
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

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

class CPlayerMessage
{
public:
	static void	SingleUserChatMessage( edict_t *pEntity, const char *szMessage );
	static void	SingleUserChatMessage( int entindex, const char *szMessage );
	static void	SingleUserChatMessage( edict_t *pEntity, const char *szMessage, const char *szPrefix );
	static void	SingleUserChatMessage( int entindex, const char *szMessage, const char *szPrefix );
	static void	AllUserChatMessage( const char *szMessage );
	static void	AllUserChatMessage( const char *szMessage, const char *szPrefix );
	static void	AllUserHudReset();

	static void	AllUserHudMsg( const char *szMessage, colour rgba, float timeonscreen, float x = -1, float y = -1, int channel = 1 );
	static void	AllUserHudHintText( const char *szMessage );

	static void	SingleUserVGUIMenu( int clientIndex, const char *title, const char *url, bool bVisible = true );
	static void	SingleUserEmptyVGUIMenu( int clientIndex );
};

#endif // PLAYER_MESSAGE_H

