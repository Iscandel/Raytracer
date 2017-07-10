#pragma once
#include "Integrator.h"

class Parameters;

class PathTracingMIS :
	public Integrator
{
public:
	PathTracingMIS(const Parameters& params);
	~PathTracingMIS();

	Color li(Scene & scene, Sampler::ptr sampler, const Ray & ray) override;

protected:
	LightSamplingStrategy myStrategy;

	int myMaxDepth;

	int myAOLength;
};

