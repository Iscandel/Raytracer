#pragma once
#include "Integrator.h"

#include "Parameters.h"

class NormalIntegrator :
	public Integrator
{
public:
	NormalIntegrator(const Parameters& params);
	~NormalIntegrator();

	Color li(Scene& scene, Sampler::ptr sampler, const Ray& ray, RadianceType::ERadianceType radianceType = RadianceType::ALL_RADIANCE) override;
};

