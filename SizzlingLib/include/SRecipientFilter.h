
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/
#ifndef _SRECIPIENT_FILTER_H
#define _SRECIPIENT_FILTER_H

#include "irecipientfilter.h"
//#include "bitvec.h"
//#include "tier1/utlvector.h"

class SRecipientFilter : public IRecipientFilter		// a version of MRecipientFilter
{														// that is optimized for 1 recipient
public:
	SRecipientFilter(int iPlayer = 0): m_Recipient(iPlayer) {};
	~SRecipientFilter(void) {};

	virtual bool	IsReliable( void ) const { return false; }
	virtual bool	IsInitMessage( void ) const { return false; }

	virtual int		GetRecipientCount( void ) const { return 1; };
	virtual int		GetRecipientIndex( int slot ) const;

	void			SetRecipient(int iPlayer);

private:
	int m_Recipient;
};

#endif // _SRECIPIENT_FILTER_H
