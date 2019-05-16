#pragma once
#include "PhaseFunction.h"

#include "core/Parameters.h"

class IsotropicPhaseFunction :
	public PhaseFunction
{
public:
	IsotropicPhaseFunction(const Parameters&);
	~IsotropicPhaseFunction();

	real sample(PhaseFunctionSamplingInfos &infos, const Point2d &sample) const override;

	/**
	* \brief Evaluate the phase function for a pair of directions
	* specified in \code pRec
	*
	* \param infos
	*     A record with detailed information on the PHASE query
	* \return
	*     The phase function value, evaluated for each color channel
	*/
	real eval(const PhaseFunctionSamplingInfos &infos) const override;

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
	real pdf(const PhaseFunctionSamplingInfos &infos) const override;
};

