#include "Medium.h"

#include "IsotropicPhaseFunction.h"
#include "HGPhaseFunction.h"

Medium::Medium(const Parameters& params)
{
	myPhaseFunction = params.getPhaseFunction("phaseFunction", PhaseFunction::ptr(new IsotropicPhaseFunction()));//Parameters())));
}


Medium::~Medium()
{
}

double Medium::samplePF(PhaseFunctionSamplingInfos & infos, const Point2d & sample) const
{
	return myPhaseFunction->sample(infos, sample);
}

double Medium::evalPF(const PhaseFunctionSamplingInfos & infos) const
{
	return myPhaseFunction->eval(infos);
}

double Medium::pdfPF(const PhaseFunctionSamplingInfos & infos) const
{
	return myPhaseFunction->pdf(infos);
}
