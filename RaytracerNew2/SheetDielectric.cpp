#include "SheetDielectric.h"

#include "DifferentialGeometry.h"
#include "Fresnel.h"
#include "ObjectFactoryManager.h"
#include "ConstantTexture.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
SheetDielectric::SheetDielectric(const Parameters& params)
{
	myReflectanceTexture = params.getTexture("reflectanceTexture", Texture::ptr(new ConstantTexture(Color(1.f))));
	myTransmittedTexture = params.getTexture("transmittedTexture", Texture::ptr(new ConstantTexture(Color(1.f))));

	myEtaI = params.getReal("etaExt", 1.000277f); //incident
	myEtaT = params.getReal("etaInt", 1.5046f); //transmitted
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
SheetDielectric::~SheetDielectric()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color SheetDielectric::eval(const BSDFSamplingInfos &)
{
	//Same as conductors & specular BSDF. The proba to have wo corresponding to 
	//refracted wi is close to 0. Return a lazy (?) 0. (see pbrt book ? The 
	//integral evaluates to 0 for specular ?)
	return Color(0.);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
//Color SheetDielectric::sample(BSDFSamplingInfos & infos, const Point2d & sample)
//{
//	real cosThetaI = std::abs(DifferentialGeometry::cosTheta(infos.wi));
//	
//	bool wasNeg = DifferentialGeometry::cosTheta(infos.wi) < 0. ? true : false;
//	infos.wi.z() = std::abs(infos.wi.z());
//	real etaI = myEtaI;
//	real etaT = myEtaT;
//	real fr = 0.;
//
//	infos.measure = Measure::DISCRETE;
//
//	real relativeEta = etaI / etaT;
//
//	//Descartes's law
//	real sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosThetaI * cosThetaI));
//	//Trigo law
//	real cosThetaT = std::sqrt(std::max(0., 1 - sinThetaT * sinThetaT));
//
//	if (sinThetaT >= 1.)
//	{
//		fr = 1.;
//	}
//	else
//	{
//		//We want "aperture" of angle, so put abs()
//		fr = fresnel::fresnelDielectric(etaI, etaT, std::abs(cosThetaI), cosThetaT);
//		cosThetaT = -cosThetaT;
//	}
//
//	infos.relativeEta = 1.;
//
//	//Reflection
//	if (sample.x() <= fr)
//	{
//		infos.sampledType = BSDF::DELTA_REFLECTION;	
//		if (wasNeg) {
//			infos.wi.z() = -infos.wi.z();
//		}
//		infos.wo = reflect(infos.wi);
//
//		infos.pdf = fr;
//
//		//brdf is the same as it is for conductors : fr / cos(theta)
//		return myReflectanceTexture->eval(infos.uv);// * fr;
//	}
//	else
//	{
//		infos.sampledType = BSDF::GLOSSY_TRANSMISSION;
//		infos.wo = refract(infos.wi, cosThetaT, relativeEta);
//
//		//refract again
//		infos.wo = -infos.wo;
//		infos.wo.z() = -infos.wo.z();
//		cosThetaI = DifferentialGeometry::cosTheta(infos.wo);
//		relativeEta = 1. / relativeEta;
//		//std::cout << sinThetaT << " " << cosThetaT << std::endl;
//		//Descartes's law
//		sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosThetaI * cosThetaI));
//		//Trigo law
//		cosThetaT = std::sqrt(std::max(0., 1 - sinThetaT * sinThetaT));
//		//std::cout << cosThetaI << " " << cosThetaT << " " << sinThetaT << infos.wo << std::endl;
//		infos.wo = refract(infos.wo, cosThetaT, relativeEta);
//
//		if (wasNeg) {
//			infos.wi.z() = -infos.wi.z();
//			//infos.wo.z() = -infos.wo.z();
//		}
//		else {
//			infos.wo.z() = -infos.wo.z();
//		}
//			
//		//infos.wo.normalize(); //?
//		infos.pdf = 1 - fr;
//
//		//For "sheet-large" glass meshes, there is no exit point, so no decompression and the 
//		//glass darkens the area it covers.
//		return myTransmittedTexture->eval(infos.uv);
//	}
//}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color SheetDielectric::sample(BSDFSamplingInfos & infos, const Point2d & sample)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	real etaI = myEtaI;
	real etaT = myEtaT;
	real fr = 0.;

	infos.measure = Measure::DISCRETE;

	real relativeEta = etaI / etaT;

	//Descartes's law
	real sinThetaT = relativeEta * std::sqrt(std::max((real)0., 1 - cosThetaI * cosThetaI));
	//Trigo law
	real cosThetaT = std::sqrt(std::max((real)0., 1 - sinThetaT * sinThetaT));

	//if (sinThetaT >= 1.)
	//{
	//	fr = 1.;
	//}
	//else
	{
		//We want "aperture" of angle, so put abs()
		fr = fresnel::fresnelDielectric(etaI, etaT, std::abs(cosThetaI), cosThetaT);
		real T = 1.f - fr;
		if (fr < 1.f)
			fr += T * T * fr / (1.f - fr * fr);
	}

	infos.relativeEta = 1.f;

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
		infos.sampledType = BSDF::UNKNOWN;//::DELTA_TRANSMISSION;
		infos.wo = -infos.wi;
		//infos.wo.normalize(); //?
		infos.pdf = 1 - fr;

		//For "sheet-large" glass meshes, there is no exit point, so no decompression and the 
		//glass darkens the area it covers.
		return myTransmittedTexture->eval(infos.uv);
	}
}



//=============================================================================
///////////////////////////////////////////////////////////////////////////////
real SheetDielectric::pdf(const BSDFSamplingInfos &)
{
	//Check if infos.wo corresponds to infos.wi, given infos.sampledType ? Would be correct
	//to avoid returning lazy 0. pdf. We could call pdf() everywhere

	return 0.0;
}

RT_REGISTER_TYPE(SheetDielectric, BSDF)