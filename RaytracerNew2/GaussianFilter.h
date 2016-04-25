#pragma once
#include "ReconstructionFilter.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief Gaussian filter. Nice results, but may introduce blurring.
///////////////////////////////////////////////////////////////////////////////
class GaussianFilter :
	public ReconstructionFilter
{
public:
	GaussianFilter(double radiusX, double radiusY, double a, double w);
	GaussianFilter(const Parameters& params);
	~GaussianFilter(void);

	double getValue(double x, double y);

	double func1D(double val);

protected:
	double myAlpha;
	double myOmega;
};

