#include "Dielectric.h"

#include "DifferentialGeometry.h"
#include "Fresnel.h"
#include "ObjectFactoryManager.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Dielectric::Dielectric(const Parameters& params)
{
	myEtaI = params.getDouble("etaExt", 1.000277); //incident
	myEtaT = params.getDouble("etaInt", 1.5046); //transmitted
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Dielectric::~Dielectric()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color Dielectric::eval(const BSDFSamplingInfos & infos)
{
	return Color(0.);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color Dielectric::sample(BSDFSamplingInfos & infos, const Point2d & sample)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	double etaI = myEtaI;
	double etaT = myEtaT;
	bool isEntering = true;
	double fr = 0.;
	
	infos.measure = Measure::DISCRETE;

	if (cosThetaI <= 0.)
	{
		isEntering = false;
		std::swap(etaI, etaT);
	}

	double relativeEta = etaI / etaT;

	//Descartes's law
	double sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosThetaI * cosThetaI));
	//Trigo law
	double cosThetaT = std::sqrt(std::max(0., 1 - sinThetaT * sinThetaT));

	if (sinThetaT >= 1.)
	{
		fr = 1.;
	}
	else
	{
		//We want "aperture" of angle, so put abs()
		fr = fresnel::fresnelDielectric(etaI, etaT, std::abs(cosThetaI), cosThetaT);
		if (isEntering)
			cosThetaT = -cosThetaT;
	}

	infos.relativeEta = relativeEta;

	//Reflection
	if (sample.x() <= fr)
	{
		infos.sampledType = BSDF::DELTA_REFLECTION;
		infos.wo = reflect(infos.wi);
		infos.pdf = fr;
		
		//divide by cos theta ?
		return Color(1.);// / fr;
	}
	else
	{
		infos.sampledType = BSDF::DELTA_TRANSMISSION;
		infos.wo = refract(infos.wi, cosThetaT, relativeEta);
		//infos.wo.normalize(); //?
		infos.pdf = 1 - fr;
	
		//btdf is eta * eta * (1 - fr), which is transmitted energy
		//divide by cos theta ?
		return Color(1.) * relativeEta * relativeEta;// / (1. - fr);
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double Dielectric::pdf(const BSDFSamplingInfos & infos)
{
	//Check if infos.wo corresponds to infos.wi, given infos.sampledType ? Would be correct
	//to avoid returning lazy 0. pdf. We could call pdf() everywhere

	return 0.0;
}

RT_REGISTER_TYPE(Dielectric, BSDF)