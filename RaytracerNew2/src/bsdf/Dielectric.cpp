#include "Dielectric.h"

#include "core/DifferentialGeometry.h"
#include "core/Fresnel.h"
#include "factory/ObjectFactoryManager.h"
#include "texture/ConstantTexture.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Dielectric::Dielectric(const Parameters& params)
	:BSDF(params)
{
	myReflectanceTexture = params.getTexture("reflectanceTexture", Texture::ptr(new ConstantTexture(Color(1.))));
	myTransmittedTexture = params.getTexture("transmittedTexture", Texture::ptr(new ConstantTexture(Color(1.))));

	myEtaI = params.getReal("etaExt", 1.000277f); //incident
	myEtaT = params.getReal("etaInt", 1.5046f); //transmitted
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Dielectric::~Dielectric()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color Dielectric::eval(const BSDFSamplingInfos &)
{
	//Same as conductors & specular BSDF. The proba to have wo corresponding to 
	//refracted wi is close to 0. Return a lazy (?) 0. (see pbrt book ? The 
	//integral evaluates to 0 for specular ?)
	return Color(0.);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color Dielectric::sample(BSDFSamplingInfos & infos, const Point2d & sample)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	real etaI = myEtaI;
	real etaT = myEtaT;
	bool isEntering = true;
	real fr = 0.;
	
	infos.measure = Measure::DISCRETE;

	if (cosThetaI <= 0.)
	{
		isEntering = false;
		std::swap(etaI, etaT);
	}

	real relativeEta = etaI / etaT;

	//Descartes's law
	real sinThetaT = relativeEta * std::sqrt(std::max((real)0., 1 - cosThetaI * cosThetaI));
	//Trigo law
	real cosThetaT = std::sqrt(std::max((real)0., 1 - sinThetaT * sinThetaT));

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
		return myTransmittedTexture->eval(infos.uv) * relativeEta * relativeEta;// * (1. - fr);
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
real Dielectric::pdf(const BSDFSamplingInfos &)
{
	//Check if infos.wo corresponds to infos.wi, given infos.sampledType ? Would be correct
	//to avoid returning lazy 0. pdf. We could call pdf() everywhere

	return 0.0;
}

RT_REGISTER_TYPE(Dielectric, BSDF)