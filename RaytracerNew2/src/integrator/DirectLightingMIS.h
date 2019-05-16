#pragma once
#include "Integrator.h"

#include "core/Parameters.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief Mix between direct integrator and path tracer. Results are biaised.
///////////////////////////////////////////////////////////////////////////////
class DirectLightingMIS :
	public Integrator
{
public:
	DirectLightingMIS(const Parameters& params);
	~DirectLightingMIS();

	Color li(Scene& scene, Sampler::ptr, const Ray& ray, RadianceType::ERadianceType radianceType = RadianceType::ALL_RADIANCE) override;

	real balanceHeuristic(real nSampleFirst, real pdfFirst, real nSampleSec, real pdfSec)
	{
		return (nSampleFirst * pdfFirst) / (nSampleFirst * pdfFirst + nSampleSec * pdfSec);
	}

	real powerHeuristic(real nSampleFirst, real pdfFirst, real nSampleSec, real pdfSec)
	{
		real left = (nSampleFirst * pdfFirst) * (nSampleFirst * pdfFirst);
		real right = (nSampleSec * pdfSec) * (nSampleSec * pdfSec);
		return left / (left + right);
	}

	real powerHeuristic(int nSampleFirst, real pdfFirst, int nSampleSec, real pdfSec, real beta)
	{
		return std::pow(nSampleFirst * pdfFirst, beta) / std::pow(nSampleFirst * pdfFirst + nSampleSec * pdfSec, beta);
	}

protected:
	int myDepth;
	int mySampleNumber;
};

