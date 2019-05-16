#include "HGPhaseFunction.h"

#include "Math.h"
#include "factory/ObjectFactoryManager.h"

HGPhaseFunction::HGPhaseFunction(const Parameters& params)
{
	myG = params.getReal("g", -0.8f);
}


HGPhaseFunction::~HGPhaseFunction()
{
}

/**
* \brief Sample the phase function and return the importance weight (i.e. the
* value of the phase function divided by the probability density of the sample
* with respect to solid angles).
*
* \param pRec    A phase function query record
* \param sample  A uniformly distributed sample on \f$[0,1]^2\f$
*
* \return The phase function value divided by the probability density of the sample
*         sample. A zero value means that sampling failed.
*/
real HGPhaseFunction::sample(PhaseFunctionSamplingInfos &infos, const Point2d &sample) const
{
	real cosTheta;
	if (myG < math::EPSILON)
		cosTheta = 1.f - 2.f * sample.x();
	else
		cosTheta = (1 + myG * myG - ((1 - myG * myG) * (1 - myG * myG) / (1 - myG + 2 * myG * sample.x()))) / (2 * myG);
	real theta = std::acos(cosTheta);

	real phi = 2 * math::PI * sample.y();

	infos.wo = cartesianFromSpherical(theta, phi);
	infos.pdf = pdf(infos);

	return 1.;//eval(infos) / pdf(infos);
}

/**
* \brief Evaluate the phase function for a pair of directions
* specified in \code pRec
*
* \param infos
*     A record with detailed information on the PHASE query
* \return
*     The phase function value, evaluated for each color channel
*/
real HGPhaseFunction::eval(const PhaseFunctionSamplingInfos &infos) const
{
	real cosTheta = (-infos.wi).dot(infos.wo);
	return math::INV_FOUR_PI * (1 - myG * myG) / std::pow(1 + myG * myG - 2 * myG * cosTheta, 1.5f);
}

/**
* \brief Compute the probability of sampling \c pRec.wo
* (conditioned on \c pRec.wi).
*
* This method provides access to the probability density that
* is realized by the \ref sample() method.
*
* \param infos
*     A record with detailed information on the PHASE query
*
* \return
*     A probability/density value expressed with respect
*     to the solid angle measure
*/

real HGPhaseFunction::pdf(const PhaseFunctionSamplingInfos &infos) const
{
	return eval(infos);
	//real cosTheta = (-infos.wi).dot(infos.wo);
	//return (1 - myG * myG) / (2 * std::pow(1 + myG * myG - 2 * myG * cosTheta, 1.5));
}

RT_REGISTER_TYPE(HGPhaseFunction, PhaseFunction)