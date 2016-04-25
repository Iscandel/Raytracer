#include "MitchellNetravaliFilter.h"

#include "ObjectFactoryManager.h"

MitchellNetravaliFilter::MitchellNetravaliFilter(double radiusX, double radiusY, double B, double C)
:ReconstructionFilter(radiusX, radiusY)
,myB(B)
,myC(C)
{
	precompute();
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
MitchellNetravaliFilter::MitchellNetravaliFilter(const Parameters& params)
: ReconstructionFilter(params)
{
	myB = params.getDouble("B", 1. / 3);
	myC = params.getDouble("C", 1. / 3);

	precompute();
}


//=============================================================================
///////////////////////////////////////////////////////////////////////////////
MitchellNetravaliFilter::~MitchellNetravaliFilter(void)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double MitchellNetravaliFilter::getValue(double x, double y)
{
	return func1D(x / myRadiusX) * func1D(y / myRadiusY);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double MitchellNetravaliFilter::func1D(double val)
{
	val = std::abs(2. * val);

	if(val > 1.)
	{
		return ((-myB - 6 * myC) * val * val * val 
			+ (6 * myB + 30 * myC) * val * val 
			+ (-12 * myB - 48 * myC) * val
			+ 8 * myB + 24 * myC) * (1. / 6.);
	}
	else
	{
		return ((12 - 9 * myB - 6 * myC) * val * val * val 
			+ (-18 + 12 * myB + 6 * myC) * val * val
			+ (6 - 2 * myB)) * (1. / 6.);
	}
}

RT_REGISTER_TYPE(MitchellNetravaliFilter, ReconstructionFilter)