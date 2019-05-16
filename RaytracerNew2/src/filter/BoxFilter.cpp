#include "BoxFilter.h"

#include "factory/ObjectFactoryManager.h"


BoxFilter::BoxFilter(real radiusX, real radiusY)
:ReconstructionFilter(radiusX, radiusY)
{
	precompute();
}

BoxFilter::BoxFilter(const Parameters& params)
: ReconstructionFilter(params)
{
	precompute();
}


BoxFilter::~BoxFilter(void)
{
}

RT_REGISTER_TYPE(BoxFilter, ReconstructionFilter)