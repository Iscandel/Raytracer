#pragma once
#include "Integrator.h"

#include "Parameters.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief Mix between direct integrator and path tracer. Results are biaised.
///////////////////////////////////////////////////////////////////////////////
class DirectLightingMIS :
	public Integrator
{
public:
	DirectLightingMIS(const Parameters& params);
	~DirectLightingMIS();

	Color li(Scene& scene, Sampler::ptr, const Ray& ray) override;

	double balanceHeuristic(double nSampleFirst, double pdfFirst, double nSampleSec, double pdfSec)
	{
		return (nSampleFirst * pdfFirst) / (nSampleFirst * pdfFirst + nSampleSec * pdfSec);
	}

	double powerHeuristic(double nSampleFirst, double pdfFirst, double nSampleSec, double pdfSec)
	{
		double left = (nSampleFirst * pdfFirst) * (nSampleFirst * pdfFirst);
		double right = (nSampleSec * pdfSec) * (nSampleSec * pdfSec);
		return left / (left + right);
	}

	double powerHeuristic(int nSampleFirst, double pdfFirst, int nSampleSec, double pdfSec, double beta)
	{
		return std::pow(nSampleFirst * pdfFirst, beta) / std::pow(nSampleFirst * pdfFirst + nSampleSec * pdfSec, beta);
	}

protected:
	int myDepth;
	int mySampleNumber;
};

