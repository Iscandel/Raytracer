#include "SheetDielectric.h"

#include "DifferentialGeometry.h"
#include "Fresnel.h"
#include "ObjectFactoryManager.h"
#include "ConstantTexture.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
SheetDielectric::SheetDielectric(const Parameters& params)
{
	myReflectanceTexture = params.getTexture("reflectanceTexture", Texture::ptr(new ConstantTexture(Color(1.))));
	myTransmittedTexture = params.getTexture("transmittedTexture", Texture::ptr(new ConstantTexture(Color(1.))));

	myEtaI = params.getDouble("etaExt", 1.000277); //incident
	myEtaT = params.getDouble("etaInt", 1.5046); //transmitted
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
//	double cosThetaI = std::abs(DifferentialGeometry::cosTheta(infos.wi));
//	
//	bool wasNeg = DifferentialGeometry::cosTheta(infos.wi) < 0. ? true : false;
//	infos.wi.z() = std::abs(infos.wi.z());
//	double etaI = myEtaI;
//	double etaT = myEtaT;
//	double fr = 0.;
//
//	infos.measure = Measure::DISCRETE;
//
//	double relativeEta = etaI / etaT;
//
//	//Descartes's law
//	double sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosThetaI * cosThetaI));
//	//Trigo law
//	double cosThetaT = std::sqrt(std::max(0., 1 - sinThetaT * sinThetaT));
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
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	double etaI = myEtaI;
	double etaT = myEtaT;
	double fr = 0.;

	infos.measure = Measure::DISCRETE;

	double relativeEta = etaI / etaT;

	//Descartes's law
	double sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosThetaI * cosThetaI));
	//Trigo law
	double cosThetaT = std::sqrt(std::max(0., 1 - sinThetaT * sinThetaT));

	//if (sinThetaT >= 1.)
	//{
	//	fr = 1.;
	//}
	//else
	{
		//We want "aperture" of angle, so put abs()
		fr = fresnel::fresnelDielectric(etaI, etaT, std::abs(cosThetaI), cosThetaT);
		double T = 1. - fr;
		if (fr < 1.)
			fr += T * T * fr / (1. - fr * fr);
	}

	infos.relativeEta = 1.;

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
double SheetDielectric::pdf(const BSDFSamplingInfos &)
{
	//Check if infos.wo corresponds to infos.wi, given infos.sampledType ? Would be correct
	//to avoid returning lazy 0. pdf. We could call pdf() everywhere

	return 0.0;
}

RT_REGISTER_TYPE(SheetDielectric, BSDF)