#include "IsotropicPhaseFunction.h"

#include "Mapping.h"
#include "ObjectFactoryManager.h"
#include "Tools.h"

IsotropicPhaseFunction::IsotropicPhaseFunction(const Parameters&)
{
}


IsotropicPhaseFunction::~IsotropicPhaseFunction()
{
}

double IsotropicPhaseFunction::sample(PhaseFunctionSamplingInfos &infos, const Point2d &sample) const
{
	infos.wo = Mapping::squareToUniformSphere(sample);
	infos.pdf = pdf(infos);

	//eval() / pdf()
	return 1.;
}

double IsotropicPhaseFunction::eval(const PhaseFunctionSamplingInfos &) const
{
	return tools::INV_FOUR_PI;
}

double IsotropicPhaseFunction::pdf(const PhaseFunctionSamplingInfos &) const
{
	return tools::INV_FOUR_PI;
}

RT_REGISTER_TYPE(IsotropicPhaseFunction, PhaseFunction)