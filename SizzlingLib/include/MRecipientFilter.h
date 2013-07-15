
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/
#ifndef _MRECIPIENT_FILTER_H
#define _MRECIPIENT_FILTER_H

#include "irecipientfilter.h"
#include "tier1/utlvector.h"

class MRecipientFilter : public IRecipientFilter
{
public:
	MRecipientFilter(void) {};
	~MRecipientFilter(void) {};

	virtual bool	IsReliable( void ) const { return false; }
	virtual bool	IsInitMessage( void ) const { return false; }

	virtual int		GetRecipientCount( void ) const;
	virtual int		GetRecipientIndex( int slot ) const;

	void			AddAllPlayers();
	void			AddRecipient(int iPlayer);

private:
	CUtlVector<int> m_Recipients;
};

class CSizzPluginContext;

class MRecipientFilter_new: public IRecipientFilter
{
public:
	MRecipientFilter_new( CSizzPluginContext &context );

	virtual bool IsReliable( void ) const { return false; }
	virtual bool IsInitMessage( void ) const { return false; }

	virtual int GetRecipientCount( void ) const;
	virtual int GetRecipientIndex( int slot ) const;

	void AddAllPlayers();
	void AddRecipient(int ent_index);

private:
	CSizzPluginContext *m_context;
	CUtlVector<char> m_recipients;
};

#endif
