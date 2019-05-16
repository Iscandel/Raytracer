#pragma once
#include "Integrator.h"

#include "medium/Homogeneous.h"

class VolPathTracing :
	public Integrator
{
public:
	VolPathTracing(const Parameters& params);
	~VolPathTracing();

	Color li(Scene & scene, Sampler::ptr sampler, const Ray & ray, RadianceType::ERadianceType radianceType = RadianceType::ALL_RADIANCE) override;
	Color li(Scene & scene, Sampler::ptr sampler, const Ray & ray, int depth, const Intersection& inter, Color& throughput, real& eta);

	Color evalTransmittanceFromSampledBSDF(Scene& scene, const Ray& ray, Sampler::ptr sampler, 
																	Medium::ptr medium, Intersection& inter, bool& wasIntersected, Medium::ptr& newMedium);

protected:
	LightSamplingStrategy myStrategy;

	int myMinDepth;
	int myMaxDepth;

	int myAOLength;

	//Homogeneous myMedium;
};

