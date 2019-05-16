#pragma once

#include "core/Color.h"
#include "light/Light.h"
#include "PhaseFunction.h"
#include "core/Ray.h"
#include "sampler/Sampler.h"
#include "core/Transform.h"
//#include "WithSmartPtr.h"

#include <memory>

class Parameters;

class Medium : public Light//, public WithSmartPtr<Medium>
{
public:
	using ptr = WithSmartPtr<Medium>::ptr;
//public:
//	typedef std::shared_ptr<Medium> ptr;

public:
	Medium(const Parameters& params);
	~Medium();

	virtual bool sampleDistance(const Ray& ray, Sampler::ptr sample, real &t, Color &weight, Color& emissivity) = 0;

	virtual Color transmittance(const Ray& ray, Sampler::ptr sampler) = 0;


	real samplePF(PhaseFunctionSamplingInfos &pRec, const Point2d &sample) const;

	real evalPF(const PhaseFunctionSamplingInfos &infos) const;

	real pdfPF(const PhaseFunctionSamplingInfos &infos) const;

	virtual void setOwnerBBox(const BoundingBox& ) 
	{
	}

	virtual bool isEmissive() const = 0;

protected:
	PhaseFunction::ptr myPhaseFunction;

	Transform::ptr myWorldBoxToUnitBox;
	BoundingBox myOwnerBox;
};

