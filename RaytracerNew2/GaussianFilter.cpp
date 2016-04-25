#include "GaussianFilter.h"

#include "ObjectFactoryManager.h"

GaussianFilter::GaussianFilter(double radiusX, double radiusY, double a, double w)
:ReconstructionFilter(radiusX, radiusY)
,myAlpha(a)
,myOmega(w)
{
	precompute();
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
GaussianFilter::GaussianFilter(const Parameters& params)
: ReconstructionFilter(params)
{
	myAlpha = params.getDouble("alpha", 1 / (2. * 0.5 * 0.5));
	myOmega = params.getDouble("omega", myRadiusX);

	precompute();
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
GaussianFilter::~GaussianFilter(void)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double GaussianFilter::getValue(double x, double y)
{
	return func1D(x) * func1D(y);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double GaussianFilter::func1D(double val)
{
	return std::max(0., std::exp(-myAlpha * val * val) - std::exp(-myAlpha * myOmega * myOmega));
}

RT_REGISTER_TYPE(GaussianFilter, ReconstructionFilter)
