
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef STATIC_SIZE_VECTOR_H
#define STATIC_SIZE_VECTOR_H

#include "platform.h"

template <typename T, int K>
class CStaticSizeVector
{
public:
	CStaticSizeVector()
	{
	}

	~CStaticSizeVector()
	{
	}

	void PlaceElement( int index, T elem )
	{
		assert( index >= 0 && index <= K );
		m_Array[index] = elem;
	}

private:
	T m_Array[K];
};

#endif // STATIC_SIZE_VECTOR_H
