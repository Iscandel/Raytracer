#pragma once

#include "Integrator.h"
#include "Parameters.h"

class DirectLighting : public Integrator
{
public:
	DirectLighting(const Parameters& params);
	~DirectLighting();

	Color li(Scene& scene, Sampler::ptr sampler, const Ray& ray) override;

protected:
	int myDepth;
};

