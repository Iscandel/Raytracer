#pragma once
#include "Integrator.h"
#include "Parameters.h"

class AmbientOcclusion :
	public Integrator
{
public:
	AmbientOcclusion(const Parameters& params);
	~AmbientOcclusion();

	Color li(Scene& scene, Sampler::ptr sampler, const Ray& ray) override;

protected:
	double myRayLength;
};

