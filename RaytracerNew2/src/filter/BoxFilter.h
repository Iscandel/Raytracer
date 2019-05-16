#pragma once
#include "ReconstructionFilter.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief Simple mean from affected values. May introduce undesirable aliasing
///////////////////////////////////////////////////////////////////////////////
class BoxFilter :
	public ReconstructionFilter
{
public:
	BoxFilter() {}
	BoxFilter(const Parameters& params);
	BoxFilter(real radiusX, real radiusY);
	~BoxFilter(void);

	real getValue(real, real) override {return 1.;}
};

