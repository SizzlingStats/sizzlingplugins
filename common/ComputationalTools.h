#ifndef COMPUTATIONAL_TOOLS
#define COMPUTATIONAL_TOOLS

#include <minmax.h>

template <typename T, int N>
class MovingAverage
{
	enum { val = (N >= 1) & !(N & (N - 1)) };
	static_assert(val, "The Sample size template parameter should be a power of 2");

	public:
		MovingAverage() : num_samples_(0), total_(0) {}

	void Reset()
	{
		num_samples_ = 0;
		total_ = 0;
	}

	void AddNumber( T sample )
	{
		if (num_samples_ < N)
		{
			samples_[num_samples_++] = sample;
			total_ += sample;
		} else {
			T& oldest = samples_[num_samples_++ % N];
			total_ += sample - oldest;
			oldest = sample;
		}
	}

	operator double() const { return total_ / min(num_samples_, N); }

	private:
		T samples_[N];
		int num_samples_;
		T total_;
};
#endif
