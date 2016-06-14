#pragma once
#include "Integrator.h"
#include "Parameters.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief Light or BSDF importance sampling
///////////////////////////////////////////////////////////////////////////////
class DirectLightingIS :
	public Integrator
{
public:
	DirectLightingIS(const Parameters& params);
	~DirectLightingIS();

	Color li(Scene& scene, Sampler::ptr sampler, const Ray& ray) override;

protected:
	int myDepth;
	int mySampleNumber;
	bool myIsBSDFSampling;
};
