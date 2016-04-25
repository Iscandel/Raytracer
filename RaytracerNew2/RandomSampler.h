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

	double getNextSample1D();

	//Override
	Sampler::ptr clone() override;

protected:
	Rng myRng;
};

