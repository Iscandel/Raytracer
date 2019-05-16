#include "Medium.h"

#include "IsotropicPhaseFunction.h"
#include "HGPhaseFunction.h"

#include "Parameters.h"

Medium::Medium(const Parameters& params)
{
	myPhaseFunction = params.getPhaseFunction("phaseFunction", PhaseFunction::ptr(new IsotropicPhaseFunction(params)));//Parameters())));
}


Medium::~Medium()
{
}

real Medium::samplePF(PhaseFunctionSamplingInfos & infos, const Point2d & sample) const
{
	return myPhaseFunction->sample(infos, sample);
}

real Medium::evalPF(const PhaseFunctionSamplingInfos & infos) const
{
	return myPhaseFunction->eval(infos);
}

real Medium::pdfPF(const PhaseFunctionSamplingInfos & infos) const
{
	return myPhaseFunction->pdf(infos);
}
