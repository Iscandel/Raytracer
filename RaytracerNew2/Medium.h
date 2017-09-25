#pragma once

#include "Color.h"
#include "PhaseFunction.h"
#include "Ray.h"
#include "Sampler.h"

#include <memory>

class Parameters;

class Medium
{
public:
	typedef std::shared_ptr<Medium> ptr;

public:
	Medium(const Parameters& params);
	~Medium();

	virtual bool sampleDistance(const Ray& ray, Sampler::ptr sample, double &t, Color &weight) = 0;

	virtual Color transmittance(const Ray& ray, Sampler::ptr sampler) = 0;


	double samplePF(PhaseFunctionSamplingInfos &pRec, const Point2d &sample) const;

	double evalPF(const PhaseFunctionSamplingInfos &infos) const;

	double pdfPF(const PhaseFunctionSamplingInfos &infos) const;

protected:
	PhaseFunction::ptr myPhaseFunction;
};

