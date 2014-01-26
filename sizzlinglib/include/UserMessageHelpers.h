
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef USER_MESSAGE_HELPERS
#define USER_MESSAGE_HELPERS

#include <stdarg.h>
#include "MRecipientFilter.h"
#include "SRecipientFilter.h"

class CSizzPluginContext;
typedef struct hud_msg_cfg_s hud_msg_cfg_t;
typedef struct motd_msg_cfg_s motd_msg_cfg_t;

class CUserMessageHelpers
{
public:
	CUserMessageHelpers( CSizzPluginContext *context );

	void SingleUserChatMessage( int ent_index, const char *format, ... );
	void SingleUserChatMessageArg( int ent_index, const char *format, va_list args );

	void AllUserChatMessage( const char *format, ... );
	void AllUserChatMessageArg( const char *format, va_list args );

	void SingleUserHudResetMessage( int ent_index );
	void AllUserHudResetMessage();

	void SingleUserHudMessage( int ent_index, const hud_msg_cfg_t &cfg, const char *format, ... );
	void SingleUserHudMessageArg( int ent_index, const hud_msg_cfg_t &cfg, const char *format, va_list args );

	void AllUserHudMessage( const hud_msg_cfg_t &cfg, const char *format, ... );
	void AllUserHudMessageArg( const hud_msg_cfg_t &cfg, const char *format, va_list args );

	void SingleUserHudHintMessage( int ent_index, const char *format, ... );
	void SingleUserHudHintMessageArg( int ent_index, const char *format, va_list args );

	void AllUserHudHintMessage( const char *format, ... );
	void AllUserHudHintMessageArg( const char *format, va_list args );

	void SingleUserMOTDPanelMessage( int ent_index, const char *msg, const motd_msg_cfg_t &cfg );
	void AllUserMOTDPanelMessage( const char *msg, const motd_msg_cfg_t &cfg );

private:
	CUserMessageHelpers( const CUserMessageHelpers &other );
	CUserMessageHelpers &operator=( const CUserMessageHelpers &other );

private:
	CSizzPluginContext *m_context;
	MRecipientFilter m_multi_recip;
	SRecipientFilter m_single_recip;
};

#endif // USER_MESSAGE_HELPERS
