#include "LayeredBSDF.h"

#include "factory/ObjectFactoryManager.h"

#include "core/Fresnel.h"
#include "core/DifferentialGeometry.h"
#include "texture/ConstantTexture.h"
#include "core/Math.h"



#include "RoughConductor.h"
//#include "RoughDielectric.h"

#include <algorithm>

LayeredBSDF::LayeredBSDF(const Parameters& params)
	: myThickness(1.2f)
	, myDistribution("ggx")
{
	myThickness = params.getReal("thickness", 1.2f);
	myAbsorbance = params.getColor("absorbance", Color(0.f));
	myEtaExt = params.getReal("etaExt", 1.000277f); //incident
	myEtaInt = params.getReal("etaInt", 1.5046f); //transmitted
	std::string distrib = params.getString("distribution", "ggx");
	//myTopBSDF = params.getBSDF("topBSDF", BSDF::ptr(new RoughDielectric(Parameters())));
	myBaseBSDF = params.getBSDF("baseBSDF", BSDF::ptr(new RoughConductor(Parameters())));
	myAlphaTop = params.getReal("alphaTop", 0.01f);
	myDistribution = MicrofacetDistribution(distrib);

	//Parameters topParams;
	//topParams.addReal("alpha", myAlphaTop);
	//topParams.addString("distribution", distrib);
	//myTopBSDF = BSDF::ptr(new RoughDielectric(topParams));
}

LayeredBSDF::~LayeredBSDF()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color LayeredBSDF::fresnel(real etaExt, real etaInt, real cosThetaI, real& etaI, real& etaT, real& relativeEta, real& cosThetaT)
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

Color LayeredBSDF::eval(const BSDFSamplingInfos & infos)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	//real alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;

	//Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
	Vector3d wh = (infos.wi + infos.wo).normalized();
	real etaI, etaT, relativeEta, cosThetaT;
	real fr1 = fresnel::estimateDielectric(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).average();
	//real fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r();
	real T12 = 1.f - fr1;

	real G = myDistribution.G(infos.wi, infos.wo, wh, myAlphaTop);

	if (fr1 == 1.)
		return Color();
	Vector3d refractedWi = refract(infos.wi, Normal3d(wh), relativeEta, cosThetaT);
	Vector3d refractedWo = refract(infos.wo, Normal3d(wh), relativeEta, cosThetaT);

	refractedWi.normalize();
	refractedWo.normalize();

	//real alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
	Color f1 = evalReflection(infos, Color(fr1), myAlphaTop);

	refractedWi = -refractedWi;
	refractedWo = -refractedWo;
	wh = (refractedWi + refractedWo).normalized();

	real fr2 = fresnel::estimateDielectric(myEtaExt, myEtaInt, (-refractedWo).dot(wh), etaI, etaT, relativeEta, cosThetaT).average();
	//real fr2= fresnel(myEtaExt, myEtaInt, (-refractedWo).dot(wh), etaI, etaT, relativeEta, cosThetaT).r();
	//real fr2 = fresnel(myEtaInt, myEtaExt, (-refractedWo).dot(wh), etaI, etaT, relativeEta, cosThetaT).r; // ?? ok ??
	if (fr2 == 1.f)
		return Color();
	real T21 = 1.f - fr2;

	//Color fr2 = fresnel::fresnelConductor(myEta, myAbsorption, DifferentialGeometry::cosTheta(refractedWi));
	BSDFSamplingInfos refractedInfos = infos;
	refractedInfos.wi = refractedWi;
	refractedInfos.wo = refractedWo;
	//alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaBase;
	relativeEta = myEtaExt / myEtaInt;
	Color f2 = myBaseBSDF->eval(refractedInfos) *relativeEta * relativeEta * DifferentialGeometry::cosTheta(infos.wo) / DifferentialGeometry::cosTheta(refractedInfos.wo);//evalReflection(refractedInfos, fr2, alpha);

	real wiPDotNormal = refractedWi.z();
	real woPDotNormal = refractedWo.z();

	//Color a(std::exp(-myAbsorbance.r * myThickness * (1.f / std::abs(wiPDotNormal) + 1.f / std::abs(woPDotNormal))),
	//	std::exp(-myAbsorbance.g * myThickness * (1.f / std::abs(wiPDotNormal) + 1.f / std::abs(woPDotNormal))),
	//	std::exp(-myAbsorbance.b * myThickness * (1.f / std::abs(wiPDotNormal) + 1.f / std::abs(woPDotNormal))));
	Color a = math::fastExp(-myAbsorbance * myThickness * (1.f / std::abs(wiPDotNormal) + 1.f / std::abs(woPDotNormal)));

	//real G = myDistribution.G(refractedWi, refractedWo, wh, myAlphaTop);

	real t = (1.f - G) + T21 * G;
	return f1 + T12 * f2 * a * t;
}



Color LayeredBSDF::evalReflection(const BSDFSamplingInfos & infos, const Color& fr, real alpha)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	Vector3d wh = (infos.wi + infos.wo).normalized();
	wh = math::sign<real>(cosThetaI) * wh;

	Color term = myDistribution.D(wh, alpha) * fr *
		myDistribution.G(infos.wi, infos.wo, wh, alpha) / (4 * cosThetaI * cosThetaO);
	//if (term.r < 0)
	//	return -1 * term;
	return term;
}

Color LayeredBSDF::sample(BSDFSamplingInfos & infos, const Point2d & sample)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	real alpha = (1.2f - 0.2f * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
	Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.nextReal(), myRng.nextReal()), alpha);//.random(0., 1.), myRng.random(0., 1.)), alpha);

	real etaI, etaT, relativeEta, cosThetaT;
	real fr1 = fresnel::estimateDielectric(myEtaExt, myEtaInt, infos.wi.dot(normal), etaI, etaT, relativeEta, cosThetaT).average();
	//real fr1 = fresnel(myEtaExt, myEtaInt,infos.wi.dot(normal), etaI, etaT, relativeEta, cosThetaT).r();

	if (fr1 == 1.f)
		return Color();

	if (sample.x() <= fr1)
	{
		infos.wo = reflect(infos.wi, normal);
		infos.pdf = pdf(infos);// *fr1;
		infos.sampledType = BSDF::GLOSSY_REFLECTION;
		infos.measure = Measure::SOLID_ANGLE;	
		Color f1 = evalReflection(infos, Color(fr1), alpha);
		if (f1.isZero())
			return Color();
		real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
		//return f1 * cosThetaO / infos.pdf;
	}
	else
	{
		infos.sampledType = BSDF::GLOSSY_REFLECTION;
		infos.measure = Measure::SOLID_ANGLE;
		
		

		Vector3d refractedWi = -refract(infos.wi, normal, relativeEta, cosThetaT);
		refractedWi.normalize();

		BSDFSamplingInfos refractedInfos(refractedWi);
		Color f2 = myBaseBSDF->sample(refractedInfos, sample);
		if (f2.isZero())
			return Color();

		real T12 = 1.f - fr1;

		infos.wo = -refractedInfos.wo;
		//infos.pdf = refractedInfos.pdf * (1. - fr1);

		real wiPDotNormal = DifferentialGeometry::cosTheta(refractedWi);
		real woPDotNormal = DifferentialGeometry::cosTheta(-refractedInfos.wo);

		//Color a(std::exp(-myAbsorbance.r * myThickness * (1.f / std::abs(wiPDotNormal) + 1.f / std::abs(woPDotNormal))),
		//	std::exp(-myAbsorbance.g * myThickness * (1.f / std::abs(wiPDotNormal) + 1.f / std::abs(woPDotNormal))),
		//	std::exp(-myAbsorbance.b * myThickness * (1.f / std::abs(wiPDotNormal) + 1.f / std::abs(woPDotNormal))));
		Color a(math::fastExp(-myAbsorbance * myThickness * (1.f / std::abs(wiPDotNormal) + 1.f / std::abs(woPDotNormal))));

		cosThetaI = DifferentialGeometry::cosTheta(infos.wo);
		real fr2 = fresnel::estimateDielectric(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).average();
		//real fr2 = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r();
		if (fr2 == 1.f)
			return Color();
		real T21 = 1.f - fr2;

		
		//f1 = myTopBSDF->sample(infos, sample);

		infos.wo = refract(infos.wo, normal, relativeEta, cosThetaT);
		infos.wo.normalize();
		//refractedInfos = BSDFSamplingInfos(infos.wo);
		//refractedWi = refract(refractedInfos.wi, normal, relativeEta, cosThetaT);
		//refractedWi.normalize();
		Color f1 = Color();//evalReflection(infos, Color(fr2), myAlphaTop);
		//infos.wo = refractedWi;
		infos.pdf = pdf(infos);// *(1. - fr1);
		real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

		if (infos.pdf == 0.)
			return Color();
		Color res = (f1 + T12 * f2 * a * T21) *std::abs(cosThetaO) / (1.f - fr1);//infos.pdf;
		
		//return res;

		
	}

	if (infos.pdf == 0.)
		return Color();
	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
	return eval(infos) * std::abs(cosThetaO) / infos.pdf;
}

real LayeredBSDF::pdf(const BSDFSamplingInfos & infos)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	Vector3d wh = (infos.wi + infos.wo).normalized();

	real etaI, etaT, relativeEta, cosThetaT;
	real fr1 = fresnel::estimateDielectric(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).average();
	//real fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r();

	if (fr1 == 1.)
		return 0.;
	Vector3d refractedWi = refract(infos.wi, Normal3d(wh), relativeEta, cosThetaT);
	Vector3d refractedWo = refract(infos.wo, Normal3d(wh), relativeEta, cosThetaT);

	refractedWi.normalize();
	refractedWo.normalize();

	//

	real cosThetaH = DifferentialGeometry::cosTheta(wh);

	real Jh = 1.f / (4 * std::abs(wh.dot(infos.wo)));
	real D = myDistribution.D(wh, myAlphaTop);
	real pdfDielectric = std::abs(D * cosThetaH * Jh);

	refractedWi = -refractedWi;
	refractedWo = -refractedWo;

	//
	BSDFSamplingInfos refractedInfos = infos;//(refractedWi, refractedWo);
	refractedInfos.wi = refractedWi;
	refractedInfos.wo = refractedWo;

	real pdfConductor = myBaseBSDF->pdf(refractedInfos);

	real weight = 0.5f;
	//return weight * pdfDielectric + (1 - weight) * pdfConductor;
	relativeEta = myEtaExt / myEtaInt;
	return pdfDielectric * fr1 + pdfConductor * (1.f - fr1) *relativeEta * relativeEta * DifferentialGeometry::cosTheta(infos.wo) / DifferentialGeometry::cosTheta(refractedInfos.wo);
}



















//V3 : Ok, but bad for smooth conductors
//LayeredBSDF::LayeredBSDF(const Parameters& params)
//	: myThickness(1.2)
//	, myAbsorbance(0.1, 0.7, 0.7)
//	, myDistribution("ggx")
//{
//	myThickness = params.getReal("thickness", 1.2);
//	myAbsorbance = params.getColor("absorbance", Color(0.1, 0.7, 0.7));
//	myEtaExt = params.getReal("etaExt", 1.000277); //incident
//	myEtaInt = params.getReal("etaInt", 1.5046); //transmitted
//	std::string distrib = params.getString("distribution", "ggx");
//	//myTopBSDF = params.getBSDF("topBSDF", BSDF::ptr(new RoughDielectric(Parameters())));
//	myBaseBSDF = params.getBSDF("baseBSDF", BSDF::ptr(new RoughConductor(Parameters())));
//	myAlphaTop = params.getReal("alphaTop", 0.01);
//	myDistribution = MicrofacetDistribution(distrib);
//
//	//Parameters topParams;
//	//topParams.addReal("alpha", myAlphaTop);
//	//topParams.addString("distribution", distrib);
//	//myTopBSDF = BSDF::ptr(new RoughDielectric(topParams));
//}
//
//LayeredBSDF::~LayeredBSDF()
//{
//}
//
////=============================================================================
/////////////////////////////////////////////////////////////////////////////////
//Color LayeredBSDF::fresnel(real etaExt, real etaInt, real cosThetaI, real& etaI, real& etaT, real& relativeEta, real& cosThetaT)
//{
//	etaI = etaExt;
//	etaT = etaInt;
//	bool isEntering = true;
//	Color fr(0.);
//
//	if (cosThetaI <= 0.)
//	{
//		isEntering = false;
//		std::swap(etaI, etaT);
//	}
//	relativeEta = etaI / etaT;
//
//	real sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosThetaI * cosThetaI));
//	cosThetaT = std::sqrt(std::max(0., 1 - sinThetaT * sinThetaT));
//
//	if (sinThetaT >= 1.)
//	{
//		cosThetaT = 0.;
//		fr = Color(1.);
//	}
//	else
//	{
//		//We want "aperture" of angle, so put abs()
//		fr = fresnel::fresnelDielectric(Color(etaI), Color(etaT), std::abs(cosThetaI), cosThetaT);
//		if (isEntering)
//			cosThetaT = -cosThetaT;
//	}
//
//	return fr;
//}
//
//Color LayeredBSDF::eval(const BSDFSamplingInfos & infos)
//{
//	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	//real alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
//
//	//Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
//	Vector3d wh = (infos.wi + infos.wo).normalized();
//	real etaI, etaT, relativeEta, cosThetaT;
//	real fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
//	real T12 = 1. - fr1;
//
//	real G = myDistribution.G(infos.wi, infos.wo, wh, myAlphaTop);
//
//	if (fr1 == 1.)
//		return Color();
//	Vector3d refractedWi = refract(infos.wi, Normal3d(wh), relativeEta, cosThetaT);
//	Vector3d refractedWo = refract(infos.wo, Normal3d(wh), relativeEta, cosThetaT);
//
//	refractedWi.normalize();
//	refractedWo.normalize();
//
//	//real alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
//	Color f1 = evalReflection(infos, Color(fr1), myAlphaTop);
//
//	refractedWi = -refractedWi;
//	refractedWo = -refractedWo;
//	wh = (refractedWi + refractedWo).normalized();
//
//	real fr2 = fresnel(myEtaExt, myEtaInt, (-refractedWo).dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
//	if (fr2 == 1.)
//		return Color();
//	real T21 = 1. - fr2;
//
//	//Color fr2 = fresnel::fresnelConductor(myEta, myAbsorption, DifferentialGeometry::cosTheta(refractedWi));
//	BSDFSamplingInfos refractedInfos = infos;
//	refractedInfos.wi = refractedWi;
//	refractedInfos.wo = refractedWo;
//	//alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaBase;
//	Color f2 = myBaseBSDF->eval(refractedInfos);//evalReflection(refractedInfos, fr2, alpha);
//
//	real wiPDotNormal = refractedWi.z();
//	real woPDotNormal = refractedWo.z();
//
//	Color a(std::exp(-myAbsorbance.r * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//		std::exp(-myAbsorbance.g * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//		std::exp(-myAbsorbance.b * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))));
//
//	//real G = myDistribution.G(refractedWi, refractedWo, wh, myAlphaTop);
//
//	real t = (1. - G) + T21 * G;
//	return f1 + T12 * f2 * a * t;
//}
//
//
//
//Color LayeredBSDF::evalReflection(const BSDFSamplingInfos & infos, const Color& fr, real alpha)
//{
//	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
//
//	Vector3d wh = (infos.wi + infos.wo).normalized();
//	wh = tools::sign(cosThetaI) * wh;
//
//	Color term = myDistribution.D(wh, alpha) * fr *
//		myDistribution.G(infos.wi, infos.wo, wh, alpha) / (4 * cosThetaI * cosThetaO);
//	//if (term.r < 0)
//	//	return -1 * term;
//	return term;
//}
//
//Color LayeredBSDF::sample(BSDFSamplingInfos & infos, const Point2d & sample)
//{
//	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	real alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
//	Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
//
//	real etaI, etaT, relativeEta, cosThetaT;
//	real fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(normal), etaI, etaT, relativeEta, cosThetaT).r;
//
//
//
//	if (sample.x() <= fr1)
//	{
//		infos.wo = reflect(infos.wi, normal);
//		infos.pdf = pdf(infos) * fr1;
//		infos.sampledType = BSDF::GLOSSY_REFLECTION;
//		infos.measure = Measure::SOLID_ANGLE;
//		real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);	
//		Color f1 = evalReflection(infos, Color(fr1), alpha);
//		if (f1.isZero())
//			return Color();
//		return f1 * cosThetaO / infos.pdf;
//	}
//	else
//	{
//		infos.sampledType = BSDF::GLOSSY_REFLECTION;
//		infos.measure = Measure::SOLID_ANGLE;
//
//		Vector3d refractedWi = -refract(infos.wi, normal, relativeEta, cosThetaT);
//		refractedWi.normalize();
//
//		BSDFSamplingInfos refractedInfos(refractedWi);
//		Color f2 = myBaseBSDF->sample(refractedInfos, sample);
//
//		real T12 = 1. - fr1;
//
//		infos.wo = -refractedInfos.wo;
//		//infos.pdf = refractedInfos.pdf * (1. - fr1);
//
//		real wiPDotNormal = DifferentialGeometry::cosTheta(refractedWi);
//		real woPDotNormal = DifferentialGeometry::cosTheta(-refractedInfos.wo);
//
//		//Color a(std::exp(-myAbsorbance.r * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//		//	std::exp(-myAbsorbance.g * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//		//	std::exp(-myAbsorbance.b * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))));
//
//		cosThetaI = DifferentialGeometry::cosTheta(infos.wo);
//		real fr2 = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r;
//		if (fr2 == 1.)
//			return Color();
//		//real T21 = 1. - fr2;
//
//		//f1 = myTopBSDF->sample(infos, sample);
//
//		infos.wo = refract(infos.wo, normal, relativeEta, cosThetaT);
//		infos.wo.normalize();
//		//refractedInfos = BSDFSamplingInfos(infos.wo);
//		//refractedWi = refract(refractedInfos.wi, normal, relativeEta, cosThetaT);
//		//refractedWi.normalize();
//
//		//infos.wo = refractedWi;
//		infos.pdf = pdf(infos) * (1. - fr1);
//		real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
//
//
//		//Color res = (f1 + T12 * f2 * a * T21) *std::abs(cosThetaO) / infos.pdf;
//
//		//return res;
//
//
//	}
//
//	if (infos.pdf == 0.)
//		return Color();
//	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
//	return eval(infos) * std::abs(cosThetaO) / infos.pdf;
//}
//
//real LayeredBSDF::pdf(const BSDFSamplingInfos & infos)
//{
//	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	Vector3d wh = (infos.wi + infos.wo).normalized();
//
//	real etaI, etaT, relativeEta, cosThetaT;
//	real fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
//
//	if (fr1 == 1.)
//		return 0.;
//	Vector3d refractedWi = refract(infos.wi, Normal3d(wh), relativeEta, cosThetaT);
//	Vector3d refractedWo = refract(infos.wo, Normal3d(wh), relativeEta, cosThetaT);
//
//	refractedWi.normalize();
//	refractedWo.normalize();
//
//	//
//
//	real cosThetaH = DifferentialGeometry::cosTheta(wh);
//
//	real Jh = 1. / (4 * std::abs(wh.dot(infos.wo)));
//	real D = myDistribution.D(wh, myAlphaTop);
//	real pdfDielectric = std::abs(D * cosThetaH * Jh);
//
//	refractedWi = -refractedWi;
//	refractedWo = -refractedWo;
//
//	//
//	BSDFSamplingInfos refractedInfos = infos;//(refractedWi, refractedWo);
//	refractedInfos.wi = refractedWi;
//	refractedInfos.wo = refractedWo;
//
//	real pdfConductor = myBaseBSDF->pdf(refractedInfos);
//
//	real weight = 0.5;
//	return weight * pdfDielectric + (1 - weight) * pdfConductor;
//}













//No
//LayeredBSDF::LayeredBSDF(const Parameters& params)
//	: myThickness(1.2)
//	, myAbsorbance(0.1, 0.7, 0.7)
//	, myDistribution("ggx")
//{
//	myThickness = params.getReal("thickness", 1.2);
//	myAbsorbance = params.getColor("absorbance", Color(0.1, 0.7, 0.7));
//	myEtaExt = params.getReal("etaExt", 1.000277); //incident
//	myEtaInt = params.getReal("etaInt", 1.5046); //transmitted
//	std::string distrib = params.getString("distribution", "ggx");
//	//myTopBSDF = params.getBSDF("topBSDF", BSDF::ptr(new RoughDielectric(Parameters())));
//	myBaseBSDF = params.getBSDF("baseBSDF", BSDF::ptr(new RoughConductor(Parameters())));
//	myAlphaTop = params.getReal("alphaTop", 0.01);
//	myDistribution = MicrofacetDistribution(distrib);
//
//	Parameters topParams;
//	topParams.addReal("alpha", myAlphaTop);
//	topParams.addString("distribution", distrib);
//	myTopBSDF = BSDF::ptr(new RoughDielectric(topParams));
//}
//
//LayeredBSDF::~LayeredBSDF()
//{
//}
//
////=============================================================================
/////////////////////////////////////////////////////////////////////////////////
//Color LayeredBSDF::fresnel(real etaExt, real etaInt, real cosThetaI, real& etaI, real& etaT, real& relativeEta, real& cosThetaT)
//{
//	etaI = etaExt;
//	etaT = etaInt;
//	bool isEntering = true;
//	Color fr(0.);
//
//	if (cosThetaI <= 0.)
//	{
//		isEntering = false;
//		std::swap(etaI, etaT);
//	}
//	relativeEta = etaI / etaT;
//
//	real sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosThetaI * cosThetaI));
//	cosThetaT = std::sqrt(std::max(0., 1 - sinThetaT * sinThetaT));
//
//	if (sinThetaT >= 1.)
//	{
//		cosThetaT = 0.;
//		fr = Color(1.);
//	}
//	else
//	{
//		//We want "aperture" of angle, so put abs()
//		fr = fresnel::fresnelDielectric(Color(etaI), Color(etaT), std::abs(cosThetaI), cosThetaT);
//		if (isEntering)
//			cosThetaT = -cosThetaT;
//	}
//
//	return fr;
//}
//
//Color LayeredBSDF::eval(const BSDFSamplingInfos & infos)
//{
//	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	//real alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
//
//	//Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
//	Vector3d wh = (infos.wi + infos.wo).normalized();
//	real etaI, etaT, relativeEta, cosThetaT;
//	real fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
//	real T12 = 1. - fr1;
//
//
//	if (fr1 == 1.)
//		return Color();
//	Vector3d refractedWi = refract(infos.wi, Normal3d(wh), relativeEta, cosThetaT);
//	Vector3d refractedWo = refract(infos.wo, Normal3d(wh), relativeEta, cosThetaT);
//
//	refractedWi.normalize();
//	refractedWo.normalize();
//
//	Color f1 = myTopBSDF->eval(infos);// evalReflection(infos, Color(fr1), alpha) * relativeEta * relativeEta;
//
//	refractedWi = -refractedWi;
//	refractedWo = -refractedWo;
//	wh = (refractedWi + refractedWo).normalized();
//
//	real T21 = 1. - fresnel(myEtaExt, myEtaInt, (-refractedWo).dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
//	Color fr2 = fresnel::fresnelConductor(myEta, myAbsorption, DifferentialGeometry::cosTheta(refractedWi));
//	BSDFSamplingInfos refractedInfos = infos;//(refractedWi, refractedWo);
//	refractedInfos.wi = refractedWi;
//	refractedInfos.wo = refractedWo;
//	//alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaBase;
//	Color f2 = myBaseBSDF->eval(refractedInfos);//evalReflection(refractedInfos, fr2, alpha);
//
//	real wiPDotNormal = refractedWi.z();
//	real woPDotNormal = refractedWo.z();
//
//	Color a(std::exp(-myAbsorbance.r * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//		std::exp(-myAbsorbance.g * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//		std::exp(-myAbsorbance.b * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))));
//
//	real G = myDistribution.G(refractedWi, refractedWo, wh, myAlphaTop);
//
//	real t = (1. - G) + T21 * G;
//	return f1 + T12 * f2 * a * t;
//}
//
//
//
////Color LayeredBSDF::evalReflection(const BSDFSamplingInfos & infos, const Color& fr, real alpha)
////{
////	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
////	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
////
////	Vector3d wh = (infos.wi + infos.wo).normalized();
////	wh = tools::sign(cosThetaI) * wh;
////
////	Color term = myDistribution.D(wh, alpha) * fr *
////		myDistribution.G(infos.wi, infos.wo, wh, alpha) / (4 * cosThetaI * cosThetaO);
////	//if (term.r < 0)
////	//	return -1 * term;
////	return term;
////}
//
//Color LayeredBSDF::sample(BSDFSamplingInfos & infos, const Point2d & sample)
//{
//	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	//real alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
//
//	//Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
//
//	real etaI, etaT, relativeEta, cosThetaT;
//	real fr1 = fresnel(myEtaExt, myEtaInt, cosThetaI/*infos.wi.dot(normal)*/, etaI, etaT, relativeEta, cosThetaT).r;
//
//	Color f1 = myTopBSDF->sample(infos, sample);
//	if (f1.isZero())
//		return Color();
//	if ((infos.sampledType & BSDF::REFLECTION) == 0)
//	{
//		Vector3d refractedWi = -infos.wo;//-refract(infos.wi, normal, relativeEta, cosThetaT);
//		refractedWi.normalize();
//
//		BSDFSamplingInfos refractedInfos(refractedWi);
//		Color f2 = myBaseBSDF->sample(refractedInfos, sample);
//		
//		real T12 = 1. - fr1;
//
//		infos.wo = -refractedInfos.wo;//-reflect(refractedWi, normal);
//		infos.pdf = refractedInfos.pdf * (1. - fr1);
//
//		real wiPDotNormal = DifferentialGeometry::cosTheta(refractedWi);
//		real woPDotNormal = DifferentialGeometry::cosTheta(-refractedInfos.wo);
//
//		Color a(std::exp(-myAbsorbance.r * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//			std::exp(-myAbsorbance.g * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//			std::exp(-myAbsorbance.b * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))));
//
//		cosThetaI = DifferentialGeometry::cosTheta(infos.wo);
//		real fr2 = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r;
//		if (fr2 == 1.)
//			return Color();
//		real T21 = 1. - fr2;
//
//		//f1 = myTopBSDF->sample(infos, sample);
//
//		//infos.wo = refract(infos.wo, normal, relativeEta, cosThetaT);
//		//infos.wo.normalize();
//		refractedInfos = BSDFSamplingInfos(infos.wo);
//		myTopBSDF->sample(refractedInfos, sample);
//		if ((refractedInfos.sampledType & BSDF::REFLECTION) != 0)
//			return Color();
//
//		infos.wo = refractedInfos.wo;
//
//		real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
//		
//
//		Color res = (f1 + T12 * f2 * a * T21) *std::abs(cosThetaO) / infos.pdf;
//		//infos.pdf *= refractedInfos.pdf;
//		return res;
//
//		//return eval(infos) * std::abs(cosThetaO) / infos.pdf;
//	}
//	infos.wo = -infos.wo;
//	return f1;
//}
//
//real LayeredBSDF::pdf(const BSDFSamplingInfos & infos)
//{
//	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	Vector3d wh = (infos.wi + infos.wo).normalized();
//
//	real etaI, etaT, relativeEta, cosThetaT;
//	real fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
//
//	if (fr1 == 1.)
//		return 0.;
//	Vector3d refractedWi = refract(infos.wi, Normal3d(wh), relativeEta, cosThetaT);
//	Vector3d refractedWo = refract(infos.wo, Normal3d(wh), relativeEta, cosThetaT);
//
//	refractedWi.normalize();
//	refractedWo.normalize();
//
//	//
//
//	//real cosThetaH = DifferentialGeometry::cosTheta(wh);
//
//	//real Jh = 1. / (4 * std::abs(wh.dot(infos.wo)));
//	//real D = myDistribution.D(wh, myAlphaTop);
//	real pdfDielectric = myTopBSDF->pdf(infos);//std::abs(D * cosThetaH * Jh);
//
//	refractedWi = -refractedWi;
//	refractedWo = -refractedWo;
//
//	//
//	BSDFSamplingInfos refractedInfos = infos;//(refractedWi, refractedWo);
//	refractedInfos.wi = refractedWi;
//	refractedInfos.wo = refractedWo;
//
//	real pdfConductor = myBaseBSDF->pdf(refractedInfos);
//
//	real weight = 0.5;
//	return weight * pdfDielectric + (1 - weight) * pdfConductor;
//}


















//V1 : Ok
//LayeredBSDF::LayeredBSDF(const Parameters& params)
//	:dielectric(Parameters())
//	, conductor(Parameters())
//	, myThickness(1.2)
//	, myAbsorbance(0.1, 0.7, 0.7)
//	, myDistribution("ggx")
//{
//	myEtaExt = params.getReal("etaExt", 1.000277); //incident
//	myEtaInt = params.getReal("etaInt", 1.5046); //transmitted
//	//myTopBSDF = params.getBSDF("topBSDF", RoughDielectric(Parameters()));
//	//myBaseBSDF = params.getBSDF("baseBSDF", RoughDielectric(Parameters()));
//	myAlphaTop = params.getReal("alphaTop", 0.01);
//	myAlphaBase = params.getReal("alphaBase", 0.01);
//
//	//myEta = Color(3.20, 2.78, 2.13);
//	//myAbsorption = Color(4.26, 4.19, 3.92);
//
//	myEta = Color(1.18, 0.96, 0.66);
//	myAbsorption = Color(7.04, 6.38, 5.46);
//
//	//Parameters p;
//	//p.addReal("etaExt", myEtaI);
//	//p.addReal("etaInt", myEtaT);
//	////p.addTexture("transmittedTexture", Texture::ptr(new ConstantTexture(Color(0.7, 0.1, 0.1))));
//	//p.addReal("alpha", 0.05);
//	//dielectric = RoughDielectric(p);
//
//	//p.addColor("eta", Color(0.153, 0.144, 0.136));
//	//p.addColor("absorption", Color(3.64, 3.18, 2.50));
//	//p.addReal("alpha", myAlpha);
//	//conductor = RoughConductor(p);
//}
//
//LayeredBSDF::~LayeredBSDF()
//{
//}
//
////=============================================================================
/////////////////////////////////////////////////////////////////////////////////
//Color LayeredBSDF::fresnel(real etaExt, real etaInt, real cosThetaI, real& etaI, real& etaT, real& relativeEta, real& cosThetaT)
//{
//	etaI = etaExt;
//	etaT = etaInt;
//	bool isEntering = true;
//	Color fr(0.);
//
//	if (cosThetaI <= 0.)
//	{
//		isEntering = false;
//		std::swap(etaI, etaT);
//	}
//	relativeEta = etaI / etaT;
//
//	real sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosThetaI * cosThetaI));
//	cosThetaT = std::sqrt(std::max(0., 1 - sinThetaT * sinThetaT));
//
//	if (sinThetaT >= 1.)
//	{
//		cosThetaT = 0.;
//		fr = Color(1.);
//	}
//	else
//	{
//		//We want "aperture" of angle, so put abs()
//		fr = fresnel::fresnelDielectric(Color(etaI), Color(etaT), std::abs(cosThetaI), cosThetaT);
//		if (isEntering)
//			cosThetaT = -cosThetaT;
//	}
//
//	return fr;
//}
//
//Color LayeredBSDF::eval(const BSDFSamplingInfos & infos)
//{
//	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	real alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
//
//	//Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
//	Vector3d wh = (infos.wi + infos.wo).normalized();
//	real etaI, etaT, relativeEta, cosThetaT;
//	real fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
//	real T12 = 1. - fr1;
//
//
//	if (fr1 == 1.)
//		return Color();
//	Vector3d refractedWi = refract(infos.wi, Normal3d(wh), relativeEta, cosThetaT);
//	Vector3d refractedWo = refract(infos.wo, Normal3d(wh), relativeEta, cosThetaT);
//
//	refractedWi.normalize();
//	refractedWo.normalize();
//
//	Color f1 = evalReflection(infos, Color(fr1), alpha) * relativeEta * relativeEta;
//
//	refractedWi = -refractedWi;
//	refractedWo = -refractedWo;
//	wh = (refractedWi + refractedWo).normalized();
//
//	real T21 = 1. - fresnel(myEtaExt, myEtaInt, (-refractedWo).dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
//	Color fr2 = fresnel::fresnelConductor(myEta, myAbsorption, DifferentialGeometry::cosTheta(refractedWi));
//	BSDFSamplingInfos refractedInfos(refractedWi, refractedWo);
//	alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaBase;
//	Color f2 = evalReflection(refractedInfos, fr2, alpha);
//
//	real wiPDotNormal = refractedWi.z();
//	real woPDotNormal = refractedWo.z();
//
//	Color a(std::exp(-myAbsorbance.r * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//		std::exp(-myAbsorbance.g * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//		std::exp(-myAbsorbance.b * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))));
//
//	real G = myDistribution.G(refractedWi, refractedWo, wh, alpha);
//
//	real t = (1. - G) + T21 * G;
//	return f1 + T12 * f2 * a * t;
//
//
//
//
//	//real cosI = DifferentialGeometry::cosTheta(infos.wi);
//	//real cosO = DifferentialGeometry::cosTheta(infos.wo);
//	//real etaI = myEtaI;
//	//real etaT = myEtaT;
//	//real fr = 0.;
//
//	//real relativeEta = etaI / etaT;
//	////Descartes's law
//	//real sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosI * cosI));
//	////Trigo law
//	//real cosT = std::sqrt(std::max(0., 1 - sinThetaT * sinThetaT));
//	//real t12 = 1. - fresnel::fresnelDielectric(myEtaI, myEtaT, cosI, cosT);
//
//	//Color absorbance(0.7, 0.2, 0.3);
//	//Color a(std::exp(-absorbance.r * myThickness * (1. / std::abs(cosI) + 1. / std::abs(cosO))),
//	//		std::exp(-absorbance.g * myThickness * (1. / std::abs(cosI) + 1. / std::abs(cosO))),
//	//		std::exp(-absorbance.b * myThickness * (1. / std::abs(cosI) + 1. / std::abs(cosO))));
//	//Vector3d wh = (infos.wi + infos.wo).normalized();
//	//real G = shadowingTerm(infos.wi, infos.wo, wh);
//	//real t = (1. - G) + t12 * G;
//	//return dielectric.eval(infos) + t12 * conductor.eval(infos) * a * t;
//}
//
//
//
//Color LayeredBSDF::evalReflection(const BSDFSamplingInfos & infos, const Color& fr, real alpha)
//{
//	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
//
//	Vector3d wh = (infos.wi + infos.wo).normalized();
//	wh = tools::sign(cosThetaI) * wh;
//
//	Color term = myDistribution.D(wh, alpha) * fr *
//		myDistribution.G(infos.wi, infos.wo, wh, alpha) / (4 * cosThetaI * cosThetaO);
//	//if (term.r < 0)
//	//	return -1 * term;
//	return term;
//}
//
//Color LayeredBSDF::sample(BSDFSamplingInfos & infos, const Point2d & sample)
//{
//	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	real alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
//
//	Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
//
//	real etaI, etaT, relativeEta, cosThetaT;
//	real fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(normal), etaI, etaT, relativeEta, cosThetaT).r;
//
//	if (fr1 == 1.)
//		return Color();
//
//	if (sample.x() < fr1)
//	{
//		infos.wo = reflect(infos.wi, normal);
//		infos.pdf = fr1;
//
//		return Color(1.);
//	}
//	else
//	{
//		Vector3d refractedWi = -refract(infos.wi, normal, relativeEta, cosThetaT);
//		refractedWi.normalize();
//
//		alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaBase;
//		normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
//		infos.wo = -reflect(refractedWi, normal);
//
//		real fr1b = fresnel(myEtaExt, myEtaInt, infos.wo.dot(normal), etaI, etaT, relativeEta, cosThetaT).r;
//		if (fr1b == 1.)
//		{
//			infos.pdf = 0.;
//			return Color();
//		}
//
//		infos.wo = refract(infos.wo, normal, relativeEta, cosThetaT);
//		infos.wo.normalize();
//		real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
//		infos.pdf = pdf(infos) * (1. - fr1);
//
//		if (infos.pdf <= 0.)
//			return Color();
//
//		return eval(infos) * std::abs(cosThetaO) / infos.pdf;
//	}
//
//
//	//Color res = dielectric.sample(infos, sample);
//	//if (res.isZero())
//	//	return Color();
//	//real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//	//real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
//
//	//bool reflection = cosThetaI * cosThetaO > 0;
//	//if (reflection)
//	//	return dielectric.eval(infos) * cosThetaO / dielectric.pdf(infos);
//
//	//BSDFSamplingInfos infos2 = infos;
//	//infos2.wi = infos.wo;
//	//res = conductor.sample(infos2, sample);
//	//if (res.isZero())
//	//	return Color();
//
//	//infos2.wi = infos2.wo;
//	//dielectric.sample(infos2, sample);
//	//real cosO = DifferentialGeometry::cosTheta(infos2.wo);
//	//infos = infos2;
//	//infos.pdf = pdf(infos);
//	//if (infos.pdf < 0)
//	//	return Color();
//	//return eval(infos) * cosO / infos.pdf;
//}
//
////=============================================================================
/////////////////////////////////////////////////////////////////////////////////
////real LayeredBSDF::pdfReflection(const BSDFSamplingInfos& infos, alpha)
////{
////	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
////	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
////
////	if (cosThetaI <= 0. || cosThetaO <= 0.)
////		return 0.;
////
////	Vector3d wh = (infos.wi + infos.wo).normalized();// / (infos.wi + infos.wo).squaredNorm();
////	real cosThetaH = DifferentialGeometry::cosTheta(wh);
////
////	real Jh = 1. / (4 * std::abs(wh.dot(infos.wo)));
////	real D = myDistribution.D(wh, alpha);
////	return D * cosThetaH * Jh;
////}
//
//real LayeredBSDF::pdf(const BSDFSamplingInfos & infos)
//{
//	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	Vector3d wh = (infos.wi + infos.wo).normalized();
//
//	real etaI, etaT, relativeEta, cosThetaT;
//	real fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
//
//	if (fr1 == 1.)
//		return 0.;
//	Vector3d refractedWi = refract(infos.wi, Normal3d(wh), relativeEta, cosThetaT);
//	Vector3d refractedWo = refract(infos.wo, Normal3d(wh), relativeEta, cosThetaT);
//
//	refractedWi.normalize();
//	refractedWo.normalize();
//
//	//
//
//	real cosThetaH = DifferentialGeometry::cosTheta(wh);
//
//	real Jh = 1. / (4 * std::abs(wh.dot(infos.wo)));
//	real D = myDistribution.D(wh, myAlphaTop);
//	real pdfDielectric = std::abs(D * cosThetaH * Jh);
//
//	refractedWi = -refractedWi;
//	refractedWo = -refractedWo;
//
//	//
//	wh = (refractedWi + refractedWo).normalized();
//	cosThetaH = DifferentialGeometry::cosTheta(wh);
//
//	Jh = 1. / (4 * std::abs(wh.dot(refractedWo)));
//	D = myDistribution.D(wh, myAlphaBase);
//
//	real pdfConductor = std::abs(D * cosThetaH * Jh);
//
//	real weight = 0.5;
//	return weight * pdfDielectric + (1 - weight) * pdfConductor;
//
//
//
//
//
//	//real weight = 0.5;
//	//return weight * dielectric.pdf(infos) + (1 - weight) * conductor.pdf(infos);
//}

RT_REGISTER_TYPE(LayeredBSDF, BSDF)