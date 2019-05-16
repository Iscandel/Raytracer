#include "IsotropicPhaseFunction.h"

#include "core/Mapping.h"
#include "factory/ObjectFactoryManager.h"
#include "core/Math.h"

IsotropicPhaseFunction::IsotropicPhaseFunction(const Parameters&)
{
}


IsotropicPhaseFunction::~IsotropicPhaseFunction()
{
}

real IsotropicPhaseFunction::sample(PhaseFunctionSamplingInfos &infos, const Point2d &sample) const
{
	infos.wo = Mapping::squareToUniformSphere(sample);
	infos.pdf = pdf(infos);

	//eval() / pdf()
	return 1.;
}

real IsotropicPhaseFunction::eval(const PhaseFunctionSamplingInfos &) const
{
	return math::INV_FOUR_PI;
}

real IsotropicPhaseFunction::pdf(const PhaseFunctionSamplingInfos &) const
{
	return math::INV_FOUR_PI;
}

RT_REGISTER_TYPE(IsotropicPhaseFunction, PhaseFunction)