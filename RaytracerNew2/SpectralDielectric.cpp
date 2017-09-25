#include "SpectralDielectric.h"

#include "DifferentialGeometry.h"
#include "Fresnel.h"
#include "ObjectFactoryManager.h"
#include "ConstantTexture.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
SpectralDielectric::SpectralDielectric(const Parameters& params)
{
	myReflectanceTexture = params.getTexture("reflectanceTexture", Texture::ptr(new ConstantTexture(Color(1.))));
	myTransmittedTexture = params.getTexture("transmittedTexture", Texture::ptr(new ConstantTexture(Color(1.))));

	myEtaI = params.getDouble("etaExt", 1.000277); //incident
	myEtaT = params.getColor("etaInt", Color(2.40, 2.43, 2.46)); //transmitted
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
SpectralDielectric::~SpectralDielectric()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color SpectralDielectric::eval(const BSDFSamplingInfos &)
{
	//Same as conductors & specular BSDF. The proba to have wo corresponding to 
	//refracted wi is close to 0. Return a lazy (?) 0. (see pbrt book ? The 
	//integral evaluates to 0 for specular ?)
	return Color(0.);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color SpectralDielectric::sample(BSDFSamplingInfos & infos, const Point2d & sample)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	double etaI = myEtaI;
	double etaT;

	int wavelength = 0;//myRng.random(0, 2); to change !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	switch (wavelength)
	{
	case 0: etaT = myEtaT.r; break;
	case 1: etaT = myEtaT.g; break;
	case 2: etaT = myEtaT.b; break;
	default: std::cout << "Undefined value SpectralDielectric" << std::endl;
	}

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

		//brdf is the same as it is for conductors : fr / cos(theta)
		return myReflectanceTexture->eval(infos.uv);// * fr;
	}
	else
	{
		infos.sampledType = BSDF::DELTA_TRANSMISSION;
		infos.wo = refract(infos.wi, cosThetaT, relativeEta);
		//infos.wo.normalize(); //?
		infos.pdf = 1 - fr;

		//btdf is eta * eta * (1 - fr) / cos(theta), which is transmitted energy
		//NB : Solid angle compression (eta * eta) implies that there is a compression and...
		//a decompression (relativeEta becomes 1 / relativeEta), ie light enters the glass 
		//mesh and exits in another point.
		//For "sheet-large" glass meshes, there is no exit point, so no decompression and the 
		//glass darkens the area it covers.
		Color weight = myTransmittedTexture->eval(infos.uv) * relativeEta * relativeEta;
		if (wavelength == 0)
			return Color(weight.r, 0., 0.);
		else if(wavelength == 1)
			return Color(0., weight.g, 0.);
		else if (wavelength == 2)
			return Color(0., 0., weight.b);
		else return Color(0., 0., 0.);
		//return myTransmittedTexture->eval(infos.uv) * relativeEta * relativeEta;// * (1. - fr);
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double SpectralDielectric::pdf(const BSDFSamplingInfos &)
{
	//Check if infos.wo corresponds to infos.wi, given infos.sampledType ? Would be correct
	//to avoid returning lazy 0. pdf. We could call pdf() everywhere

	return 0.0;
}

RT_REGISTER_TYPE(SpectralDielectric, BSDF)