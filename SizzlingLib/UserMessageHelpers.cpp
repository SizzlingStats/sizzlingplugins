
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "UserMessageHelpers.h"
#include "SizzPluginContext.h"
#include "SRecipientFilter.h"
#include "MRecipientFilter.h"

CUserMessageHelpers::CUserMessageHelpers( CSizzPluginContext *context ):
	m_context(context)
{
	Assert(m_context);
}

void CUserMessageHelpers::SingleUserChatMessage( int ent_index, const char *format, ... )
{
	va_list args;
	va_start(args, format);
	m_single_recip.SetRecipient(ent_index);
	m_context->ChatMessageArg(&m_single_recip, format, args);
	va_end(args);
}

void CUserMessageHelpers::SingleUserChatMessageArg( int ent_index, const char *format, va_list args )
{
	m_single_recip.SetRecipient(ent_index);
	m_context->ChatMessageArg(&m_single_recip, format, args);
}

void CUserMessageHelpers::AllUserChatMessage( const char *format, ... )
{
	va_list args;
	va_start(args, format);
	m_multi_recip.AddAllPlayers(m_context);
	m_context->ChatMessageArg(&m_multi_recip, format, args);
	va_end(args);
}

void CUserMessageHelpers::AllUserChatMessageArg( const char *format, va_list args )
{
	m_multi_recip.AddAllPlayers(m_context);
	m_context->ChatMessageArg(&m_multi_recip, format, args);
}

void CUserMessageHelpers::SingleUserHudResetMessage( int ent_index )
{
	m_single_recip.SetRecipient(ent_index);
	m_context->HudResetMessage(&m_single_recip);
}

void CUserMessageHelpers::AllUserHudResetMessage()
{
	m_multi_recip.AddAllPlayers(m_context);
	m_context->HudResetMessage(&m_multi_recip);
}

void CUserMessageHelpers::SingleUserHudMessage( int ent_index, const hud_msg_cfg_t &cfg, const char *format, ... )
{
	va_list args;
	va_start(args, format);
	m_single_recip.SetRecipient(ent_index);
	m_context->HudMessageArg(&m_single_recip, cfg, format, args);
	va_end(args);
}

void CUserMessageHelpers::SingleUserHudMessageArg( int ent_index, const hud_msg_cfg_t &cfg, const char *format, va_list args )
{
	m_single_recip.SetRecipient(ent_index);
	m_context->HudMessageArg(&m_single_recip, cfg, format, args);
}

void CUserMessageHelpers::AllUserHudMessage( const hud_msg_cfg_t &cfg, const char *format, ... )
{
	va_list args;
	va_start(args, format);
	m_multi_recip.AddAllPlayers(m_context);
	m_context->HudMessageArg(&m_multi_recip, cfg, format, args);
	va_end(args);
}

void CUserMessageHelpers::AllUserHudMessageArg( const hud_msg_cfg_t &cfg, const char *format, va_list args )
{
	m_multi_recip.AddAllPlayers(m_context);
	m_context->HudMessageArg(&m_multi_recip, cfg, format, args);
}

void CUserMessageHelpers::SingleUserHudHintMessage( int ent_index, const char *format, ... )
{
	va_list args;
	va_start(args, format);
	m_single_recip.SetRecipient(ent_index);
	m_context->HudHintMessageArg(&m_single_recip, format, args);
	va_end(args);
}

void CUserMessageHelpers::SingleUserHudHintMessageArg( int ent_index, const char *format, va_list args )
{
	m_single_recip.SetRecipient(ent_index);
	m_context->HudHintMessageArg(&m_single_recip, format, args);
}

void CUserMessageHelpers::AllUserHudHintMessage( const char *format, ... )
{
	va_list args;
	va_start(args, format);
	m_multi_recip.AddAllPlayers(m_context);
	m_context->HudHintMessageArg(&m_multi_recip, format, args);
	va_end(args);
}

void CUserMessageHelpers::AllUserHudHintMessageArg( const char *format, va_list args )
{
	m_multi_recip.AddAllPlayers(m_context);
	m_context->HudHintMessageArg(&m_multi_recip, format, args);
}

void CUserMessageHelpers::SingleUserMOTDPanelMessage( int ent_index, const char *msg, const motd_msg_cfg_t &cfg )
{
	m_single_recip.SetRecipient(ent_index);
	m_context->MOTDPanelMessage(&m_single_recip, msg, cfg);
}

void CUserMessageHelpers::AllUserMOTDPanelMessage( const char *msg, const motd_msg_cfg_t &cfg )
{
	m_multi_recip.AddAllPlayers(m_context);
	m_context->MOTDPanelMessage(&m_multi_recip, msg, cfg);
}
