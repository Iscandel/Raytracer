#include "IsotropicPhaseFunction.h"

#include "Mapping.h"
#include "Tools.h"

IsotropicPhaseFunction::IsotropicPhaseFunction()
{
}


IsotropicPhaseFunction::~IsotropicPhaseFunction()
{
}

double IsotropicPhaseFunction::sample(PhaseFunctionSamplingInfos &infos, const Point2d &sample) const
{
	infos.wo = Mapping::squareToUniformSphere(sample);

	//eval() / pdf()
	return 1.;
}

double IsotropicPhaseFunction::eval(const PhaseFunctionSamplingInfos &infos) const
{
	return tools::INV_FOUR_PI;
}

double IsotropicPhaseFunction::pdf(const PhaseFunctionSamplingInfos &infos) const
{
	return tools::INV_FOUR_PI;
}