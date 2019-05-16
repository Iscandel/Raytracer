#pragma once

#include "Integrator.h"
#include "core/Parameters.h"

class DirectLighting : public Integrator
{
public:
	DirectLighting(const Parameters& params);
	~DirectLighting();

	Color li(Scene& scene, Sampler::ptr sampler, const Ray& ray, RadianceType::ERadianceType radianceType = RadianceType::ALL_RADIANCE) override;

protected:
	int myDepth;
};

