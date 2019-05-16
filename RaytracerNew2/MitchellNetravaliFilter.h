#pragma once
#include "ReconstructionFilter.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief Filter from Mitchell and Netravali : Reconstruction filter for 
/// computer graphics.
/// Nice looking.
///////////////////////////////////////////////////////////////////////////////
class MitchellNetravaliFilter :
	public ReconstructionFilter
{
public:
	MitchellNetravaliFilter(real radiusX, real radiusY, real B, real C);
	MitchellNetravaliFilter(const Parameters& params);
	~MitchellNetravaliFilter(void);

	real getValue(real x, real y);

	real func1D(real val);

protected:
	real myB;
	real myC;
};

