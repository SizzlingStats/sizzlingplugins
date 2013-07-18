
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

CUserMessageHelpers::CUserMessageHelpers( CSizzPluginContext &context ):
	m_context(context)
{
}

void CUserMessageHelpers::SingleUserChatMessage( int ent_index, const char *format, ... )
{
	va_list args;
	va_start(args, format);
	SRecipientFilter_new filter(m_context, ent_index);
	m_context.ChatMessageArg(&filter, format, args);
	va_end(args);
}

void CUserMessageHelpers::SingleUserChatMessageArg( int ent_index, const char *format, va_list args )
{
	SRecipientFilter_new filter(m_context, ent_index);
	m_context.ChatMessageArg(&filter, format, args);
}

void CUserMessageHelpers::AllUserChatMessage( const char *format, ... )
{
	va_list args;
	va_start(args, format);
	MRecipientFilter_new filter(m_context);
	filter.AddAllPlayers();
	m_context.ChatMessageArg(&filter, format, args);
	va_end(args);
}

void CUserMessageHelpers::AllUserChatMessageArg( const char *format, va_list args )
{
	MRecipientFilter_new filter(m_context);
	filter.AddAllPlayers();
	m_context.ChatMessageArg(&filter, format, args);
}

void CUserMessageHelpers::SingleUserHudResetMessage( int ent_index )
{
	SRecipientFilter_new filter(m_context, ent_index);
	m_context.HudResetMessage(&filter);
}

void CUserMessageHelpers::AllUserHudResetMessage()
{
	MRecipientFilter_new filter(m_context);
	filter.AddAllPlayers();
	m_context.HudResetMessage(&filter);
}

void CUserMessageHelpers::SingleUserHudMessage( int ent_index, const hud_msg_cfg_t &cfg, const char *format, ... )
{
	va_list args;
	va_start(args, format);
	SRecipientFilter_new filter(m_context, ent_index);
	m_context.HudMessageArg(&filter, cfg, format, args);
	va_end(args);
}

void CUserMessageHelpers::SingleUserHudMessageArg( int ent_index, const hud_msg_cfg_t &cfg, const char *format, va_list args )
{
	SRecipientFilter_new filter(m_context, ent_index);
	m_context.HudMessageArg(&filter, cfg, format, args);
}

void CUserMessageHelpers::AllUserHudMessage( const hud_msg_cfg_t &cfg, const char *format, ... )
{
	va_list args;
	va_start(args, format);
	MRecipientFilter_new filter(m_context);
	filter.AddAllPlayers();
	m_context.HudMessageArg(&filter, cfg, format, args);
	va_end(args);
}

void CUserMessageHelpers::AllUserHudMessageArg( const hud_msg_cfg_t &cfg, const char *format, va_list args )
{
	MRecipientFilter_new filter(m_context);
	filter.AddAllPlayers();
	m_context.HudMessageArg(&filter, cfg, format, args);
}

void CUserMessageHelpers::SingleUserHudHintMessage( int ent_index, const char *format, ... )
{
	va_list args;
	va_start(args, format);
	SRecipientFilter_new filter(m_context, ent_index);
	m_context.HudHintMessageArg(&filter, format, args);
	va_end(args);
}

void CUserMessageHelpers::SingleUserHudHintMessageArg( int ent_index, const char *format, va_list args )
{
	SRecipientFilter_new filter(m_context, ent_index);
	m_context.HudHintMessageArg(&filter, format, args);
}

void CUserMessageHelpers::AllUserHudHintMessage( const char *format, ... )
{
	va_list args;
	va_start(args, format);
	MRecipientFilter_new filter(m_context);
	filter.AddAllPlayers();
	m_context.HudHintMessageArg(&filter, format, args);
	va_end(args);
}

void CUserMessageHelpers::AllUserHudHintMessageArg( const char *format, va_list args )
{
	MRecipientFilter_new filter(m_context);
	filter.AddAllPlayers();
	m_context.HudHintMessageArg(&filter, format, args);
}

void CUserMessageHelpers::SingleUserMOTDPanelMessage( int ent_index, const char *msg, const motd_msg_cfg_t &cfg )
{
	SRecipientFilter_new filter(m_context, ent_index);
	m_context.MOTDPanelMessage(&filter, msg, cfg);
}

void CUserMessageHelpers::AllUserMOTDPanelMessage( const char *msg, const motd_msg_cfg_t &cfg )
{
	MRecipientFilter_new filter(m_context);
	filter.AddAllPlayers();
	m_context.MOTDPanelMessage(&filter, msg, cfg);
}
