#pragma once

#include "Color.h"

#include <iostream>
#include <algorithm>

namespace fresnel
{
	inline Color fresnelDielectric(const Color& etaI, const Color& etaT, double cosI, double cosT)
	{
		Color etaTcosI = etaT * cosI;
		Color etaIcosT = etaI * cosT;
		Color etaIcosI = etaI * cosI;
		Color etaTcosT = etaT * cosT;

		Color rPar = (etaTcosI - etaIcosT) / (etaTcosI + etaIcosT);

		Color rPerp = (etaIcosI - etaTcosT) / (etaIcosI + etaTcosT);

		return (rPar * rPar + rPerp * rPerp) * 0.5;
	}

	inline double fresnelDielectric(double etaI, double etaT, double cosI, double cosT)
	{
		double etaTcosI = etaT * cosI;
		double etaIcosT = etaI * cosT;
		double etaIcosI = etaI * cosI;
		double etaTcosT = etaT * cosT;

		double rPar = (etaTcosI - etaIcosT) / (etaTcosI + etaIcosT);

		double rPerp = (etaIcosI - etaTcosT) / (etaIcosI + etaTcosT);

		return (rPar * rPar + rPerp * rPerp) * 0.5;
	}

	inline double fresnelConductor(double eta, double absorption, double cosI)
	{
		double etaPlusAbsorption = eta * eta + absorption * absorption;
		double cosI2 = cosI * cosI;
		double etaPlusAbsorptionCosI2 = etaPlusAbsorption * cosI2;
		double _2EtaCosI = 2 * eta * cosI;

		double rPar = (etaPlusAbsorptionCosI2 - _2EtaCosI + 1) / 
					  (etaPlusAbsorptionCosI2 + _2EtaCosI + 1);

		double rPerp = (etaPlusAbsorption - _2EtaCosI + cosI2) /
					  (etaPlusAbsorption + _2EtaCosI + cosI2);

		return (rPar * rPar + rPerp * rPerp) * 0.5;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Fresnel equations for a conductor, using multi channel indices 
	/// of refraction and absorption
	///////////////////////////////////////////////////////////////////////////
	inline Color fresnelConductor(const Color& eta, const Color& absorption, double cosI)
	{
		Color etaPlusAbsorption = eta * eta + absorption * absorption;
		double cosI2 = cosI * cosI;
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


	inline Color estimateFresnelDielectric(double etaExt, double etaInt, double cosThetaI, double& cosThetaT)
	{
		double etaI = etaExt;
		double etaT = etaInt;
		bool isEntering = true;
		Color fr(0.);

		if (cosThetaI <= 0.)
		{
			isEntering = false;
			std::swap(etaI, etaT);
		}
		double relativeEta = etaI / etaT;

		//Descartes's law
		double sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosThetaI * cosThetaI));
		//Trigo law
		cosThetaT = std::sqrt(std::max(0., 1 - sinThetaT * sinThetaT));

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
}

class Fresnel
{
public:
	Fresnel();
	~Fresnel();

	//virtual double evaluate();
};



