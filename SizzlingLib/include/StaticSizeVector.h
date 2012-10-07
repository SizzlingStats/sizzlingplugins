
#ifndef STATIC_SIZE_VECTOR_H
#define STATIC_SIZE_VECTOR_H

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
