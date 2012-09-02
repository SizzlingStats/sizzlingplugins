////////////////////////////////////////////////////////////////////////////////
// Filename: PlayerMessage.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PlayerMessage.h"
#include "MRecipientFilter.h"
#include "SRecipientFilter.h"

#include "engine/iserverplugin.h"
#include "game/server/iplayerinfo.h"
#include "eiface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Interfaces from the engine
extern IVEngineServer			*pEngine; // helper functions (messaging clients, loading content, making entities, running commands, etc)

static PlayerMessage gMessage;
PlayerMessage *g_pMessage = &gMessage;

PlayerMessage::PlayerMessage()
{
}

PlayerMessage::~PlayerMessage()
{	
}

	//	COLOURS
    //[CSS]         Print       SayText2
    //-----------------------------------
    //    \x01      Yellow      Yellow
    //    \x02      Yellow      Yellow
    //    \x03      LightGreen  Red,Blue,Grey
    //    \x04      Green       Green
    //    
    //[TF]
    //-----------------------------------
    //    \x01      White       White
    //    \x02      White       White
    //    \x03      LightGreen  Red,Blue,Grey
    //    \x04      Green       Green
    //    \x05      Olive       Olive
    //    
    //[DOD]
    //-----------------------------------
    //    \x01      Grey          -
    //    \x02      Grey          -
    //    \x03      Grey          -
    //    \x04      Green         -
    //    \x05      Olive         -

void PlayerMessage::SingleUserChatMessage( edict_t *pEntity, const char *szMessage )
{
	// Create a filter and add this client to it
	SRecipientFilter filter;
	filter.AddRecipient( pEngine->IndexOfEdict( pEntity ) );

	// Start the usermessage and get a bf_write
	bf_write *pBuffer = pEngine->UserMessageBegin( &filter, 3 );

	// Send the message
	pBuffer->WriteByte( 0 );
	pBuffer->WriteString( szMessage );
	pBuffer->WriteByte( 0 );

	// End the message
	pEngine->MessageEnd();

	return;
}

void PlayerMessage::SingleUserChatMessage( int entindex, const char *szMessage )
{
	edict_t *pEntity = pEngine->PEntityOfEntIndex( entindex );
	// Create a filter and add this client to it
	SRecipientFilter filter;
	filter.AddRecipient( pEngine->IndexOfEdict( pEntity ) );

	// Start the usermessage and get a bf_write
	bf_write *pBuffer = pEngine->UserMessageBegin( &filter, 3 );

	// Send the message
	pBuffer->WriteByte( 0 );
	pBuffer->WriteString( szMessage );
	pBuffer->WriteByte( 0 );

	// End the message
	pEngine->MessageEnd();

	return;
}

void PlayerMessage::SingleUserChatMessage( edict_t *pEntity, const char *szMessage, const char *szPrefix )
{
	// Create a filter and add this client to it
	SRecipientFilter filter;
	filter.AddRecipient( pEngine->IndexOfEdict( pEntity ) );

	// Start the usermessage and get a bf_write
	bf_write *pBuffer = pEngine->UserMessageBegin( &filter, 3 );

	// Concat the prefix message and the actual message
	char message[64] = "";
	V_snprintf( message, 64, "%s%s\n", szPrefix, szMessage );

	// Send the message
	pBuffer->WriteByte( 0 );
	pBuffer->WriteString( message );
	pBuffer->WriteByte( 0 );

	// End the message
	pEngine->MessageEnd();

	return;
}

void PlayerMessage::SingleUserChatMessage( int entindex, const char *szMessage, const char *szPrefix )
{
	edict_t *pEntity = pEngine->PEntityOfEntIndex( entindex );
	// Create a filter and add this client to it
	SRecipientFilter filter;
	filter.AddRecipient( pEngine->IndexOfEdict( pEntity ) );

	// Start the usermessage and get a bf_write
	bf_write *pBuffer = pEngine->UserMessageBegin( &filter, 3 );

	// Concat the prefix message and the actual message
	char message[64] = "";
	V_snprintf( message, 64, "%s%s\n", szPrefix, szMessage );

	// Send the message
	pBuffer->WriteByte( 0 );
	pBuffer->WriteString( message );
	pBuffer->WriteByte( 0 );

	// End the message
	pEngine->MessageEnd();

	return;
}

void PlayerMessage::AllUserChatMessage( const char *szMessage )
{
	// Create a filter and add this client to it
	MRecipientFilter filter;
	filter.AddAllPlayers();

	// Start the usermessage and get a bf_write
	bf_write *pBuffer = pEngine->UserMessageBegin( &filter, 3 );

	// Send the message
	pBuffer->WriteByte( 0 );
	pBuffer->WriteString( szMessage );
	pBuffer->WriteByte( 0 );

	// End the message
	pEngine->MessageEnd();

	return;
}

void PlayerMessage::AllUserChatMessage( const char *szMessage, const char *szPrefix )
{
	// Create a filter and add this client to it
	MRecipientFilter filter;
	filter.AddAllPlayers();

	// Start the usermessage and get a bf_write
	bf_write *pBuffer = pEngine->UserMessageBegin( &filter, 3 );

	// Concat the prefix message and the actual message
	char message[255] = "";
	V_snprintf( message, 255, "%s%s", szPrefix, szMessage );

	// Send the message
	pBuffer->WriteByte( 0 );
	pBuffer->WriteString( message );
	pBuffer->WriteByte( 0 );

	// End the message
	pEngine->MessageEnd();

	return;
}

void PlayerMessage::AllUserHudReset()
{
	MRecipientFilter filter;
	filter.AddAllPlayers();

	bf_write *pBuffer = pEngine->UserMessageBegin( &filter, 6 ); // ResetHUD: 6
		pBuffer->WriteByte( 0 );
	pEngine->MessageEnd();
}

void PlayerMessage::AllUserHudMsg( const char *szMessage, colour rgba, float timeonscreen, float x, float y, int channel )
{
	MRecipientFilter filter;
	filter.AddAllPlayers();

	bf_write *pBuffer = pEngine->UserMessageBegin( &filter, 21 ); //hudmsg: 21

		pBuffer->WriteByte( channel & 0xFF ); //TODO: find out why & 0xFF @_@

		pBuffer->WriteFloat( x );
		pBuffer->WriteFloat( y );

		pBuffer->WriteByte( rgba.r );
		pBuffer->WriteByte( rgba.g );
		pBuffer->WriteByte( rgba.b );
		pBuffer->WriteByte( rgba.a );

		pBuffer->WriteByte( rgba.r );
		pBuffer->WriteByte( rgba.g );
		pBuffer->WriteByte( rgba.b );
		pBuffer->WriteByte( rgba.a );
	
		pBuffer->WriteByte( 0 );
		pBuffer->WriteFloat( 0 );
		pBuffer->WriteFloat( 0 );

		pBuffer->WriteFloat( timeonscreen );

		pBuffer->WriteFloat( 0 );

		pBuffer->WriteString( szMessage );

	pEngine->MessageEnd();

	return;

//PerformHudMsg(client, const String:szMsg[]) {
//	new Handle:hBf = StartMessageOne("HudMsg", client)
//	BfWriteByte(hBf, 3) //channel
//	BfWriteFloat(hBf, 0.0); // x ( -1 = center )
//	BfWriteFloat(hBf, -1); // y ( -1 = center )
//	// second color
//	BfWriteByte(hBf, 0); //r1
//	BfWriteByte(hBf, 0); //g1
//	BfWriteByte(hBf, 255); //b1
//	BfWriteByte(hBf, 255); //a1 // transparent?
//	// init color
//	BfWriteByte(hBf, 255); //r2
//	BfWriteByte(hBf, 0); //g2
//	BfWriteByte(hBf, 0); //b2
//	BfWriteByte(hBf, 255); //a2
//	BfWriteByte(hBf, 0); //effect (0 is fade in/fade out; 1 is flickery credits; 2 is write out)
//	BfWriteFloat(hBf, 1.0); //fadeinTime (message fade in time - per character in effect 2)
//	BfWriteFloat(hBf, 1.0); //fadeoutTime
//	BfWriteFloat(hBf, 15.0); //holdtime
//	BfWriteFloat(hBf, 5.0); //fxtime (effect type(2) used)
//	BfWriteString(hBf, szMsg); //Message
//	EndMessage();
//}
}

void PlayerMessage::AllUserHudHintText( const char *szMessage )
{
	MRecipientFilter filter;
	filter.AddAllPlayers();

	bf_write *pBuffer = pEngine->UserMessageBegin( &filter, 20 ); //keyhinttext: 20

	pBuffer->WriteByte( 1 ); // one string
	pBuffer->WriteString( szMessage );

	pEngine->MessageEnd();

	return;
}

#define MOTDPANEL_TYPE_TEXT		"0"	/**< Treat msg as plain text */
#define MOTDPANEL_TYPE_INDEX	"1"	/**< Msg is auto determined by the engine */
#define MOTDPANEL_TYPE_URL		"2"	/**< Treat msg as an URL link */
#define MOTDPANEL_TYPE_FILE		"3"	/**< Treat msg as a filename to be openned */
          // ^^ FILE LOADS STUFF FROM THE CLIENT (bad)

void PlayerMessage::SingleUserVGUIMenu( int clientIndex, const char *title, const char *url )
{
	SRecipientFilter filter;
	filter.AddRecipient( clientIndex );

	bf_write *pBuffer = pEngine->UserMessageBegin( &filter, 12 ); //vguimenu: 12

	pBuffer->WriteString( "info" );

	// will the panel be visible? 1 is yes, 0 is no
	pBuffer->WriteByte( 1 );

	// number of entries in the following'table
	pBuffer->WriteByte( 5 );
	{
		// Title of the panel (printed on the top border of the window).
		pBuffer->WriteString( "title" );
		pBuffer->WriteString( title );

		// Determines the way to treat the message body of the panel.
		// the types are defined above
		pBuffer->WriteString( "type" );
		pBuffer->WriteString( MOTDPANEL_TYPE_URL );

		// Contents of the panel, it can be treated as an url, filename or plain text
		// depending on the type parameter (WARNING: msg has to be 192 bytes maximum!)
		pBuffer->WriteString( "msg" );							
		pBuffer->WriteString( url );

		// 0 means use a small vgui window, 1 means a large (tf2 only)
		pBuffer->WriteString( "customsvr" );
		pBuffer->WriteString( "1" );

		// what to execute after the window is closed
		//  0 for no command
		//  1 for joingame
		//  2 for changeteam
		//  3 for impulse 101
		//  4 for mapinfo
		//  5 for closed_htmlpage
		//  6 for chooseteam
		pBuffer->WriteString( "cmd" );
		pBuffer->WriteString( "5" ); 
	}
	pEngine->MessageEnd();

	return;
}

void PlayerMessage::SingleUserEmptyVGUIMenu( int clientIndex )
{
	SRecipientFilter filter;
	filter.AddRecipient( clientIndex );

	bf_write *pBuffer = pEngine->UserMessageBegin( &filter, 12 ); //vguimenu: 12

	pBuffer->WriteString( "info" );

	// will the panel be visible? 1 is yes, 0 is no
	pBuffer->WriteByte( 0 );

	// number of entries in the following 'table'
	pBuffer->WriteByte( 5 );
	{
		// Title of the panel (printed on the top border of the window).
		pBuffer->WriteString( "title" );
		pBuffer->WriteString( "" );

		// Determines the way to treat the message body of the panel.
		// the types are defined above
		pBuffer->WriteString( "type" );
		pBuffer->WriteString( MOTDPANEL_TYPE_URL );

		// Contents of the panel, it can be treated as an url, filename or plain text
		// depending on the type parameter (WARNING: msg has to be 192 bytes maximum!)
		pBuffer->WriteString( "msg" );							
		pBuffer->WriteString( "about:blank" );

		// 0 means use a small vgui window, 1 means a large (tf2 only)
		pBuffer->WriteString( "customsvr" );
		pBuffer->WriteString( "1" );

		// what to execute after the window is closed
		//  0 for no command
		//  1 for joingame
		//  2 for changeteam
		//  3 for impulse 101
		//  4 for mapinfo
		//  5 for closed_htmlpage
		//  6 for chooseteam
		pBuffer->WriteString( "cmd" );
		pBuffer->WriteString( "0" ); 
	}
	pEngine->MessageEnd();

	return;
}