
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

class CSizzPluginContext;

class MRecipientFilter: public IRecipientFilter
{
public:
	virtual bool IsReliable( void ) const { return false; }
	virtual bool IsInitMessage( void ) const { return false; }

	virtual int GetRecipientCount( void ) const;
	virtual int GetRecipientIndex( int slot ) const;

	void AddAllPlayers( CSizzPluginContext *context );
	void AddRecipient(int ent_index);

	void RemoveAll();

private:
	CUtlVector<char> m_recipients;
};

inline int MRecipientFilter::GetRecipientCount( void ) const
{
	return m_recipients.Count();
}

inline int MRecipientFilter::GetRecipientIndex( int slot ) const
{
	if (slot < 0 || slot >= GetRecipientCount())
	{
		return -1;
	}

	return m_recipients[slot];
}

inline void MRecipientFilter::AddRecipient(int ent_index)
{
	// Return if the recipient is already in the vector
	if (m_recipients.Find(ent_index) == m_recipients.InvalidIndex())
	{
		m_recipients.AddToTail(ent_index);
	}
}

inline void MRecipientFilter::RemoveAll()
{
	m_recipients.RemoveAll();
}

#endif
