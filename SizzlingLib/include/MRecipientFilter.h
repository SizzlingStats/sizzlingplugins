#ifndef _MRECIPIENT_FILTER_H
#define _MRECIPIENT_FILTER_H

#include "irecipientfilter.h"
//#include "bitvec.h"
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

#endif
