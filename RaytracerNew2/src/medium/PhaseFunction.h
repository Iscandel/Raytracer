#pragma once

#include "core/Geometry.h"

#include <memory>

struct PhaseFunctionSamplingInfos
{
	PhaseFunctionSamplingInfos(const Vector3d& _wi) : wi(_wi) {}
	PhaseFunctionSamplingInfos(const Vector3d& _wi, const Vector3d& _wo) : wi(_wi), wo(_wo) {}

	Vector3d wi;
	Vector3d wo;
	real pdf;
};

class PhaseFunction
{
public:
	typedef std::shared_ptr<PhaseFunction> ptr;

public:
	PhaseFunction();
	~PhaseFunction();

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
	virtual real sample(PhaseFunctionSamplingInfos &pRec, const Point2d &sample) const = 0;

	/**
	* \brief Evaluate the phase function for a pair of directions
	* specified in \code pRec
	*
	* \param infos
	*     A record with detailed information on the PHASE query
	* \return
	*     The phase function value, evaluated for each color channel
	*/
	virtual real eval(const PhaseFunctionSamplingInfos &infos) const = 0;

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

	virtual real pdf(const PhaseFunctionSamplingInfos &infos) const = 0;
};

