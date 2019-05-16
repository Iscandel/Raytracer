#pragma once
#include "Integrator.h"
#include "Intersection.h"
#include "Parameters.h"

class RecursivePathTracingMIS :
	public Integrator
{
public:
	RecursivePathTracingMIS(const Parameters& params);
	~RecursivePathTracingMIS();

	Color li(Scene & scene, Sampler::ptr sampler, const Ray & ray, RadianceType::ERadianceType radianceType = RadianceType::ALL_RADIANCE) override;

	Color li(Scene & scene, Sampler::ptr sampler, const Ray & ray, int depth, const Intersection& inter, Color& throughput, real& eta);



protected:
	LightSamplingStrategy myStrategy;

	int myMaxDepth;
};

