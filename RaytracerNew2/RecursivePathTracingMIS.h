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

	Color li(Scene & scene, Sampler::ptr sampler, const Ray & ray) override;

	Color li(Scene & scene, Sampler::ptr sampler, const Ray & ray, int depth, const Intersection& inter, Color& throughput, double& eta);



protected:
	LightSamplingStrategy myStrategy;

	int myMaxDepth;
};
