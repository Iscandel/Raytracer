#pragma once
#include "Integrator.h"

#include "Homogeneous.h"

class VolPathTracing :
	public Integrator
{
public:
	VolPathTracing(const Parameters& params);
	~VolPathTracing();

	Color li(Scene & scene, Sampler::ptr sampler, const Ray & ray) override;
	Color li(Scene & scene, Sampler::ptr sampler, const Ray & ray, int depth, const Intersection& inter, Color& throughput, double& eta);

protected:
	LightSamplingStrategy myStrategy;

	int myMinDepth;
	int myMaxDepth;

	int myAOLength;

	//Homogeneous myMedium;
};

