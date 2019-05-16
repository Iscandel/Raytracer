#include "GaussianFilter.h"

#include "Math.h"
#include "ObjectFactoryManager.h"

GaussianFilter::GaussianFilter(real radiusX, real radiusY, real a, real w)
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
	myAlpha = params.getReal("alpha", 1 / (2. * 0.5 * 0.5));
	myOmega = params.getReal("omega", myRadiusX);

	precompute();
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
GaussianFilter::~GaussianFilter(void)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
real GaussianFilter::getValue(real x, real y)
{
	return func1D(x) * func1D(y);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
real GaussianFilter::func1D(real val)
{
	return std::max((real)0., math::fastExp(-myAlpha * val * val) - math::fastExp(-myAlpha * myOmega * myOmega));
}

RT_REGISTER_TYPE(GaussianFilter, ReconstructionFilter)
