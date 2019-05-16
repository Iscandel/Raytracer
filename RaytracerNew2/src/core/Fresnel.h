#pragma once

#include "Color.h"

#include <iostream>
#include <algorithm>

namespace fresnel
{
	inline real fresnelConductor(real eta, real absorption, real cosI)
	{
		real etaPlusAbsorption = eta * eta + absorption * absorption;
		real cosI2 = cosI * cosI;
		real etaPlusAbsorptionCosI2 = etaPlusAbsorption * cosI2;
		real _2EtaCosI = 2 * eta * cosI;

		real rPar = (etaPlusAbsorptionCosI2 - _2EtaCosI + 1) /
			(etaPlusAbsorptionCosI2 + _2EtaCosI + 1);

		real rPerp = (etaPlusAbsorption - _2EtaCosI + cosI2) /
			(etaPlusAbsorption + _2EtaCosI + cosI2);

		return (rPar * rPar + rPerp * rPerp) * 0.5f;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Fresnel equations for a conductor, using multi channel indices 
	/// of refraction and absorption
	///////////////////////////////////////////////////////////////////////////
	inline Color fresnelConductor(const Color& eta, const Color& absorption, real cosI)
	{
		Color etaPlusAbsorption = eta * eta + absorption * absorption;
		real cosI2 = cosI * cosI;
		Color etaPlusAbsorptionCosI2 = etaPlusAbsorption * cosI2;
		Color _2EtaCosI = 2 * eta * cosI;

		Color rPar = (etaPlusAbsorptionCosI2 - _2EtaCosI + 1) /
			(etaPlusAbsorptionCosI2 + _2EtaCosI + 1);

		Color rPerp = (etaPlusAbsorption - _2EtaCosI + cosI2) /
			(etaPlusAbsorption + _2EtaCosI + cosI2);

		//if ((etaPlusAbsorptionCosI2 + _2EtaCosI + 1).luminance() == 0. || (etaPlusAbsorptionCosI2 + _2EtaCosI + 1).luminance() < 0.
		//	|| (etaPlusAbsorption + _2EtaCosI + cosI2).luminance() == 0. || (etaPlusAbsorption + _2EtaCosI + cosI2).luminance() <= 0.)
		//	std::cout << "Non !";


		return (rPar * rPar + rPerp * rPerp) * 0.5;
	}

	inline Color fresnelDielectric(const Color& etaI, const Color& etaT, real cosI, real cosT)
	{
		Color etaTcosI = etaT * cosI;
		Color etaIcosT = etaI * cosT;
		Color etaIcosI = etaI * cosI;
		Color etaTcosT = etaT * cosT;

		Color rPar = (etaTcosI - etaIcosT) / (etaTcosI + etaIcosT);

		Color rPerp = (etaIcosI - etaTcosT) / (etaIcosI + etaTcosT);

		return (rPar * rPar + rPerp * rPerp) * 0.5;
	}

	inline real fresnelDielectric(real etaI, real etaT, real cosI, real cosT)
	{
		real etaTcosI = etaT * cosI;
		real etaIcosT = etaI * cosT;
		real etaIcosI = etaI * cosI;
		real etaTcosT = etaT * cosT;

		real rPar = (etaTcosI - etaIcosT) / (etaTcosI + etaIcosT);

		real rPerp = (etaIcosI - etaTcosT) / (etaIcosI + etaTcosT);

		return (rPar * rPar + rPerp * rPerp) * 0.5f;
	}

	

	//=============================================================================
	///////////////////////////////////////////////////////////////////////////////
	inline Color estimateDielectric(real etaExt, real etaInt, real cosThetaI, real& etaI, real& etaT, real& relativeEta, real& cosThetaT)
	{
		etaI = etaExt;
		etaT = etaInt;
		bool isEntering = true;
		Color fr(0.);

		if (cosThetaI <= 0.)
		{
			isEntering = false;
			std::swap(etaI, etaT);
		}
		relativeEta = etaI / etaT;

		real sinThetaT = relativeEta * std::sqrt(std::max((real)0., 1 - cosThetaI * cosThetaI));
		cosThetaT = std::sqrt(std::max((real)0., 1 - sinThetaT * sinThetaT));

		if (sinThetaT >= 1.)
		{
			cosThetaT = 0.;
			fr = Color(1.);
		}
		else
		{
			//We want "aperture" of angle, so put abs()
			fr = fresnel::fresnelDielectric(Color(etaI), Color(etaT), std::abs(cosThetaI), cosThetaT);
			if (isEntering)
				cosThetaT = -cosThetaT;
		}

		return fr;
	}
	
	//=============================================================================
	///////////////////////////////////////////////////////////////////////////////
	inline Color estimateDielectric(real etaExt, real etaInt, real cosThetaI)
	{
		real etaI, etaT, relEta, cosT;
		return estimateDielectric(etaExt, etaInt, cosThetaI, etaI, etaT, relEta, cosT);
	}

	inline Color estimateDielectric(real etaExt, real etaInt, real cosThetaI, real& cosThetaT)
	{
		real etaI = etaExt;
		real etaT = etaInt;
		bool isEntering = true;
		Color fr(0.);

		if (cosThetaI <= 0.)
		{
			isEntering = false;
			std::swap(etaI, etaT);
		}
		real relativeEta = etaI / etaT;

		//Descartes's law
		real sinThetaT = relativeEta * std::sqrt(std::max((real)0., 1 - cosThetaI * cosThetaI));
		//Trigo law
		cosThetaT = std::sqrt(std::max((real)0., 1 - sinThetaT * sinThetaT));

		if (sinThetaT >= 1.)
		{
			fr = Color(1.);
		}
		else
		{
			//We want "aperture" of angle, so put abs()
			fr = fresnel::fresnelDielectric(Color(etaI), Color(etaT), std::abs(cosThetaI), cosThetaT);
			if (isEntering)
				cosThetaT = -cosThetaT;
		}

		return fr;
	}

	inline real estimateDiffuseReflectance(real eta)
	{
		real invEta = 1 / eta;
		return -1.440f * invEta * invEta + 0.710f * invEta + 0.668f + 0.0636f * eta;
	}
}

class Fresnel
{
public:
	Fresnel();
	~Fresnel();

	//virtual real evaluate();
};



