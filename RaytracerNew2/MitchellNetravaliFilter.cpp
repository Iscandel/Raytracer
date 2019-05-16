#include "MitchellNetravaliFilter.h"

#include "ObjectFactoryManager.h"

MitchellNetravaliFilter::MitchellNetravaliFilter(real radiusX, real radiusY, real B, real C)
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
	myB = params.getReal("B", 1.f / 3);
	myC = params.getReal("C", 1.f / 3);

	precompute();
}


//=============================================================================
///////////////////////////////////////////////////////////////////////////////
MitchellNetravaliFilter::~MitchellNetravaliFilter(void)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
real MitchellNetravaliFilter::getValue(real x, real y)
{
	return func1D(x / myRadiusX) * func1D(y / myRadiusY);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
real MitchellNetravaliFilter::func1D(real val)
{
	val = std::abs(2.f * val);

	if(val > 1.f)
	{
		return ((-myB - 6 * myC) * val * val * val 
			+ (6 * myB + 30 * myC) * val * val 
			+ (-12 * myB - 48 * myC) * val
			+ 8 * myB + 24 * myC) * (1.f / 6.f);
	}
	else
	{
		return ((12 - 9 * myB - 6 * myC) * val * val * val 
			+ (-18 + 12 * myB + 6 * myC) * val * val
			+ (6 - 2 * myB)) * (1.f / 6.f);
	}
}

RT_REGISTER_TYPE(MitchellNetravaliFilter, ReconstructionFilter)