#pragma once
#include "Integrator.h"

class Parameters;

class PathTracingMIS :
	public Integrator
{
public:
	PathTracingMIS(const Parameters& params);
	~PathTracingMIS();

	Color li(Scene & scene, Sampler::ptr sampler, const Ray & ray, RadianceType::ERadianceType radianceType = RadianceType::ALL_RADIANCE) override;

protected:
	//LightSamplingStrategy myStrategy;

	int myMinDepth;
	int myMaxDepth;

	int myAOLength;
};

