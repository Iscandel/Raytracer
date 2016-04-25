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
	BoxFilter(double radiusX, double radiusY);
	~BoxFilter(void);

	double getValue(double, double) override {return 1.;}
};

