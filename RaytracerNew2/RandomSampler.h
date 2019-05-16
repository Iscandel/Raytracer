#pragma once
#include "Sampler.h"

#include "Parameters.h"
#include "Rng.h"

class RandomSampler : public Sampler
{
public:
	RandomSampler(const Parameters& params);
	RandomSampler(int sampleNumber);
	~RandomSampler(void);

	Point2d getNextSample2D();

	real getNextSample1D();

	//Override
	Sampler::ptr clone() override;

	void seed(uint64_t s, uint64_t l)
	{
		myRng.seed(s, l);
	}

protected:
	Rng myRng;
};

