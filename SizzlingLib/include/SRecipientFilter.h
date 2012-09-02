#ifndef _SRECIPIENT_FILTER_H
#define _SRECIPIENT_FILTER_H

#include "irecipientfilter.h"
//#include "bitvec.h"
//#include "tier1/utlvector.h"

class SRecipientFilter : public IRecipientFilter		// a version of MRecipientFilter
{														// that is optimized for 1 recipient
public:
	SRecipientFilter(void) {};
	~SRecipientFilter(void) {};

	virtual bool	IsReliable( void ) const { return false; }
	virtual bool	IsInitMessage( void ) const { return false; }

	virtual int		GetRecipientCount( void ) const { return 1; };
	virtual int		GetRecipientIndex( int slot ) const;

	void			AddRecipient(int iPlayer);

private:
	int m_Recipient;
};

#endif // _SRECIPIENT_FILTER_H
