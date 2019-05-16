#pragma once
#include "ReconstructionFilter.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief Gaussian filter. Nice results, but may introduce blurring.
///////////////////////////////////////////////////////////////////////////////
class GaussianFilter :
	public ReconstructionFilter
{
public:
	GaussianFilter(real radiusX, real radiusY, real a, real w);
	GaussianFilter(const Parameters& params);
	~GaussianFilter(void);

	real getValue(real x, real y);

	real func1D(real val);

protected:
	real myAlpha;
	real myOmega;
};

