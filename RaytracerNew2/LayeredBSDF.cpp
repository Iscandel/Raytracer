#include "LayeredBSDF.h"

#include "ObjectFactoryManager.h"

#include "Fresnel.h"
#include "DifferentialGeometry.h"
#include "ConstantTexture.h"

#include "RoughConductor.h"
#include "RoughDielectric.h"

#include <algorithm>

LayeredBSDF::LayeredBSDF(const Parameters& params)
	: myThickness(1.2)
	, myAbsorbance(0.1, 0.7, 0.7)
	, myDistribution("ggx")
{
	myThickness = params.getDouble("thickness", 1.2);
	myAbsorbance = params.getColor("absorbance", Color(0.1, 0.7, 0.7));
	myEtaExt = params.getDouble("etaExt", 1.000277); //incident
	myEtaInt = params.getDouble("etaInt", 1.5046); //transmitted
	std::string distrib = params.getString("distribution", "ggx");
	//myTopBSDF = params.getBSDF("topBSDF", BSDF::ptr(new RoughDielectric(Parameters())));
	myBaseBSDF = params.getBSDF("baseBSDF", BSDF::ptr(new RoughConductor(Parameters())));
	myAlphaTop = params.getDouble("alphaTop", 0.01);
	myDistribution = MicrofacetDistribution(distrib);

	//Parameters topParams;
	//topParams.addDouble("alpha", myAlphaTop);
	//topParams.addString("distribution", distrib);
	//myTopBSDF = BSDF::ptr(new RoughDielectric(topParams));
}

LayeredBSDF::~LayeredBSDF()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color LayeredBSDF::fresnel(double etaExt, double etaInt, double cosThetaI, double& etaI, double& etaT, double& relativeEta, double& cosThetaT)
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

	double sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosThetaI * cosThetaI));
	cosThetaT = std::sqrt(std::max(0., 1 - sinThetaT * sinThetaT));

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
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	//double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;

	//Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
	Vector3d wh = (infos.wi + infos.wo).normalized();
	double etaI, etaT, relativeEta, cosThetaT;
	double fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
	double T12 = 1. - fr1;

	double G = myDistribution.G(infos.wi, infos.wo, wh, myAlphaTop);

	if (fr1 == 1.)
		return Color();
	Vector3d refractedWi = refract(infos.wi, Normal3d(wh), relativeEta, cosThetaT);
	Vector3d refractedWo = refract(infos.wo, Normal3d(wh), relativeEta, cosThetaT);

	refractedWi.normalize();
	refractedWo.normalize();

	//double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
	Color f1 = evalReflection(infos, Color(fr1), myAlphaTop);

	refractedWi = -refractedWi;
	refractedWo = -refractedWo;
	wh = (refractedWi + refractedWo).normalized();

	double fr2= fresnel(myEtaExt, myEtaInt, (-refractedWo).dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
	//double fr2 = fresnel(myEtaInt, myEtaExt, (-refractedWo).dot(wh), etaI, etaT, relativeEta, cosThetaT).r; // ?? ok ??
	if (fr2 == 1.)
		return Color();
	double T21 = 1. - fr2;

	//Color fr2 = fresnel::fresnelConductor(myEta, myAbsorption, DifferentialGeometry::cosTheta(refractedWi));
	BSDFSamplingInfos refractedInfos = infos;
	refractedInfos.wi = refractedWi;
	refractedInfos.wo = refractedWo;
	//alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaBase;
	relativeEta = myEtaExt / myEtaInt;
	Color f2 = myBaseBSDF->eval(refractedInfos) *relativeEta * relativeEta * DifferentialGeometry::cosTheta(infos.wo) / DifferentialGeometry::cosTheta(refractedInfos.wo);//evalReflection(refractedInfos, fr2, alpha);

	double wiPDotNormal = refractedWi.z();
	double woPDotNormal = refractedWo.z();

	Color a(std::exp(-myAbsorbance.r * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
		std::exp(-myAbsorbance.g * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
		std::exp(-myAbsorbance.b * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))));

	//double G = myDistribution.G(refractedWi, refractedWo, wh, myAlphaTop);

	double t = (1. - G) + T21 * G;
	return f1 + T12 * f2 * a * t;
}



Color LayeredBSDF::evalReflection(const BSDFSamplingInfos & infos, const Color& fr, double alpha)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	Vector3d wh = (infos.wi + infos.wo).normalized();
	wh = tools::sign(cosThetaI) * wh;

	Color term = myDistribution.D(wh, alpha) * fr *
		myDistribution.G(infos.wi, infos.wo, wh, alpha) / (4 * cosThetaI * cosThetaO);
	//if (term.r < 0)
	//	return -1 * term;
	return term;
}

Color LayeredBSDF::sample(BSDFSamplingInfos & infos, const Point2d & sample)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
	Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.nextFloat(), myRng.nextFloat()), alpha);//.random(0., 1.), myRng.random(0., 1.)), alpha);

	double etaI, etaT, relativeEta, cosThetaT;
	double fr1 = fresnel(myEtaExt, myEtaInt,infos.wi.dot(normal), etaI, etaT, relativeEta, cosThetaT).r;

	if (fr1 == 1.)
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
		double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
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

		double T12 = 1. - fr1;

		infos.wo = -refractedInfos.wo;
		//infos.pdf = refractedInfos.pdf * (1. - fr1);

		double wiPDotNormal = DifferentialGeometry::cosTheta(refractedWi);
		double woPDotNormal = DifferentialGeometry::cosTheta(-refractedInfos.wo);

		Color a(std::exp(-myAbsorbance.r * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
			std::exp(-myAbsorbance.g * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
			std::exp(-myAbsorbance.b * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))));

		cosThetaI = DifferentialGeometry::cosTheta(infos.wo);
		double fr2 = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r;
		if (fr2 == 1.)
			return Color();
		double T21 = 1. - fr2;

		
		//f1 = myTopBSDF->sample(infos, sample);

		infos.wo = refract(infos.wo, normal, relativeEta, cosThetaT);
		infos.wo.normalize();
		//refractedInfos = BSDFSamplingInfos(infos.wo);
		//refractedWi = refract(refractedInfos.wi, normal, relativeEta, cosThetaT);
		//refractedWi.normalize();
		Color f1 = Color();//evalReflection(infos, Color(fr2), myAlphaTop);
		//infos.wo = refractedWi;
		infos.pdf = pdf(infos);// *(1. - fr1);
		double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

		if (infos.pdf == 0.)
			return Color();
		Color res = (f1 + T12 * f2 * a * T21) *std::abs(cosThetaO) / (1. - fr1);//infos.pdf;
		
		//return res;

		
	}

	if (infos.pdf == 0.)
		return Color();
	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
	return eval(infos) * std::abs(cosThetaO) / infos.pdf;
}

double LayeredBSDF::pdf(const BSDFSamplingInfos & infos)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	Vector3d wh = (infos.wi + infos.wo).normalized();

	double etaI, etaT, relativeEta, cosThetaT;
	double fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r;

	if (fr1 == 1.)
		return 0.;
	Vector3d refractedWi = refract(infos.wi, Normal3d(wh), relativeEta, cosThetaT);
	Vector3d refractedWo = refract(infos.wo, Normal3d(wh), relativeEta, cosThetaT);

	refractedWi.normalize();
	refractedWo.normalize();

	//

	double cosThetaH = DifferentialGeometry::cosTheta(wh);

	double Jh = 1. / (4 * std::abs(wh.dot(infos.wo)));
	double D = myDistribution.D(wh, myAlphaTop);
	double pdfDielectric = std::abs(D * cosThetaH * Jh);

	refractedWi = -refractedWi;
	refractedWo = -refractedWo;

	//
	BSDFSamplingInfos refractedInfos = infos;//(refractedWi, refractedWo);
	refractedInfos.wi = refractedWi;
	refractedInfos.wo = refractedWo;

	double pdfConductor = myBaseBSDF->pdf(refractedInfos);

	double weight = 0.5;
	//return weight * pdfDielectric + (1 - weight) * pdfConductor;
	relativeEta = myEtaExt / myEtaInt;
	return pdfDielectric * fr1 + pdfConductor * (1. - fr1) *relativeEta * relativeEta * DifferentialGeometry::cosTheta(infos.wo) / DifferentialGeometry::cosTheta(refractedInfos.wo);
}



















//V3 : Ok, but bad for smooth conductors
//LayeredBSDF::LayeredBSDF(const Parameters& params)
//	: myThickness(1.2)
//	, myAbsorbance(0.1, 0.7, 0.7)
//	, myDistribution("ggx")
//{
//	myThickness = params.getDouble("thickness", 1.2);
//	myAbsorbance = params.getColor("absorbance", Color(0.1, 0.7, 0.7));
//	myEtaExt = params.getDouble("etaExt", 1.000277); //incident
//	myEtaInt = params.getDouble("etaInt", 1.5046); //transmitted
//	std::string distrib = params.getString("distribution", "ggx");
//	//myTopBSDF = params.getBSDF("topBSDF", BSDF::ptr(new RoughDielectric(Parameters())));
//	myBaseBSDF = params.getBSDF("baseBSDF", BSDF::ptr(new RoughConductor(Parameters())));
//	myAlphaTop = params.getDouble("alphaTop", 0.01);
//	myDistribution = MicrofacetDistribution(distrib);
//
//	//Parameters topParams;
//	//topParams.addDouble("alpha", myAlphaTop);
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
//Color LayeredBSDF::fresnel(double etaExt, double etaInt, double cosThetaI, double& etaI, double& etaT, double& relativeEta, double& cosThetaT)
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
//	double sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosThetaI * cosThetaI));
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
//	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	//double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
//
//	//Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
//	Vector3d wh = (infos.wi + infos.wo).normalized();
//	double etaI, etaT, relativeEta, cosThetaT;
//	double fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
//	double T12 = 1. - fr1;
//
//	double G = myDistribution.G(infos.wi, infos.wo, wh, myAlphaTop);
//
//	if (fr1 == 1.)
//		return Color();
//	Vector3d refractedWi = refract(infos.wi, Normal3d(wh), relativeEta, cosThetaT);
//	Vector3d refractedWo = refract(infos.wo, Normal3d(wh), relativeEta, cosThetaT);
//
//	refractedWi.normalize();
//	refractedWo.normalize();
//
//	//double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
//	Color f1 = evalReflection(infos, Color(fr1), myAlphaTop);
//
//	refractedWi = -refractedWi;
//	refractedWo = -refractedWo;
//	wh = (refractedWi + refractedWo).normalized();
//
//	double fr2 = fresnel(myEtaExt, myEtaInt, (-refractedWo).dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
//	if (fr2 == 1.)
//		return Color();
//	double T21 = 1. - fr2;
//
//	//Color fr2 = fresnel::fresnelConductor(myEta, myAbsorption, DifferentialGeometry::cosTheta(refractedWi));
//	BSDFSamplingInfos refractedInfos = infos;
//	refractedInfos.wi = refractedWi;
//	refractedInfos.wo = refractedWo;
//	//alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaBase;
//	Color f2 = myBaseBSDF->eval(refractedInfos);//evalReflection(refractedInfos, fr2, alpha);
//
//	double wiPDotNormal = refractedWi.z();
//	double woPDotNormal = refractedWo.z();
//
//	Color a(std::exp(-myAbsorbance.r * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//		std::exp(-myAbsorbance.g * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//		std::exp(-myAbsorbance.b * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))));
//
//	//double G = myDistribution.G(refractedWi, refractedWo, wh, myAlphaTop);
//
//	double t = (1. - G) + T21 * G;
//	return f1 + T12 * f2 * a * t;
//}
//
//
//
//Color LayeredBSDF::evalReflection(const BSDFSamplingInfos & infos, const Color& fr, double alpha)
//{
//	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
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
//	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
//	Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
//
//	double etaI, etaT, relativeEta, cosThetaT;
//	double fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(normal), etaI, etaT, relativeEta, cosThetaT).r;
//
//
//
//	if (sample.x() <= fr1)
//	{
//		infos.wo = reflect(infos.wi, normal);
//		infos.pdf = pdf(infos) * fr1;
//		infos.sampledType = BSDF::GLOSSY_REFLECTION;
//		infos.measure = Measure::SOLID_ANGLE;
//		double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);	
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
//		double T12 = 1. - fr1;
//
//		infos.wo = -refractedInfos.wo;
//		//infos.pdf = refractedInfos.pdf * (1. - fr1);
//
//		double wiPDotNormal = DifferentialGeometry::cosTheta(refractedWi);
//		double woPDotNormal = DifferentialGeometry::cosTheta(-refractedInfos.wo);
//
//		//Color a(std::exp(-myAbsorbance.r * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//		//	std::exp(-myAbsorbance.g * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//		//	std::exp(-myAbsorbance.b * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))));
//
//		cosThetaI = DifferentialGeometry::cosTheta(infos.wo);
//		double fr2 = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r;
//		if (fr2 == 1.)
//			return Color();
//		//double T21 = 1. - fr2;
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
//		double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
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
//	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
//	return eval(infos) * std::abs(cosThetaO) / infos.pdf;
//}
//
//double LayeredBSDF::pdf(const BSDFSamplingInfos & infos)
//{
//	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	Vector3d wh = (infos.wi + infos.wo).normalized();
//
//	double etaI, etaT, relativeEta, cosThetaT;
//	double fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
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
//	double cosThetaH = DifferentialGeometry::cosTheta(wh);
//
//	double Jh = 1. / (4 * std::abs(wh.dot(infos.wo)));
//	double D = myDistribution.D(wh, myAlphaTop);
//	double pdfDielectric = std::abs(D * cosThetaH * Jh);
//
//	refractedWi = -refractedWi;
//	refractedWo = -refractedWo;
//
//	//
//	BSDFSamplingInfos refractedInfos = infos;//(refractedWi, refractedWo);
//	refractedInfos.wi = refractedWi;
//	refractedInfos.wo = refractedWo;
//
//	double pdfConductor = myBaseBSDF->pdf(refractedInfos);
//
//	double weight = 0.5;
//	return weight * pdfDielectric + (1 - weight) * pdfConductor;
//}













//No
//LayeredBSDF::LayeredBSDF(const Parameters& params)
//	: myThickness(1.2)
//	, myAbsorbance(0.1, 0.7, 0.7)
//	, myDistribution("ggx")
//{
//	myThickness = params.getDouble("thickness", 1.2);
//	myAbsorbance = params.getColor("absorbance", Color(0.1, 0.7, 0.7));
//	myEtaExt = params.getDouble("etaExt", 1.000277); //incident
//	myEtaInt = params.getDouble("etaInt", 1.5046); //transmitted
//	std::string distrib = params.getString("distribution", "ggx");
//	//myTopBSDF = params.getBSDF("topBSDF", BSDF::ptr(new RoughDielectric(Parameters())));
//	myBaseBSDF = params.getBSDF("baseBSDF", BSDF::ptr(new RoughConductor(Parameters())));
//	myAlphaTop = params.getDouble("alphaTop", 0.01);
//	myDistribution = MicrofacetDistribution(distrib);
//
//	Parameters topParams;
//	topParams.addDouble("alpha", myAlphaTop);
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
//Color LayeredBSDF::fresnel(double etaExt, double etaInt, double cosThetaI, double& etaI, double& etaT, double& relativeEta, double& cosThetaT)
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
//	double sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosThetaI * cosThetaI));
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
//	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	//double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
//
//	//Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
//	Vector3d wh = (infos.wi + infos.wo).normalized();
//	double etaI, etaT, relativeEta, cosThetaT;
//	double fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
//	double T12 = 1. - fr1;
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
//	double T21 = 1. - fresnel(myEtaExt, myEtaInt, (-refractedWo).dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
//	Color fr2 = fresnel::fresnelConductor(myEta, myAbsorption, DifferentialGeometry::cosTheta(refractedWi));
//	BSDFSamplingInfos refractedInfos = infos;//(refractedWi, refractedWo);
//	refractedInfos.wi = refractedWi;
//	refractedInfos.wo = refractedWo;
//	//alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaBase;
//	Color f2 = myBaseBSDF->eval(refractedInfos);//evalReflection(refractedInfos, fr2, alpha);
//
//	double wiPDotNormal = refractedWi.z();
//	double woPDotNormal = refractedWo.z();
//
//	Color a(std::exp(-myAbsorbance.r * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//		std::exp(-myAbsorbance.g * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//		std::exp(-myAbsorbance.b * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))));
//
//	double G = myDistribution.G(refractedWi, refractedWo, wh, myAlphaTop);
//
//	double t = (1. - G) + T21 * G;
//	return f1 + T12 * f2 * a * t;
//}
//
//
//
////Color LayeredBSDF::evalReflection(const BSDFSamplingInfos & infos, const Color& fr, double alpha)
////{
////	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
////	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
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
//	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	//double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
//
//	//Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
//
//	double etaI, etaT, relativeEta, cosThetaT;
//	double fr1 = fresnel(myEtaExt, myEtaInt, cosThetaI/*infos.wi.dot(normal)*/, etaI, etaT, relativeEta, cosThetaT).r;
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
//		double T12 = 1. - fr1;
//
//		infos.wo = -refractedInfos.wo;//-reflect(refractedWi, normal);
//		infos.pdf = refractedInfos.pdf * (1. - fr1);
//
//		double wiPDotNormal = DifferentialGeometry::cosTheta(refractedWi);
//		double woPDotNormal = DifferentialGeometry::cosTheta(-refractedInfos.wo);
//
//		Color a(std::exp(-myAbsorbance.r * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//			std::exp(-myAbsorbance.g * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//			std::exp(-myAbsorbance.b * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))));
//
//		cosThetaI = DifferentialGeometry::cosTheta(infos.wo);
//		double fr2 = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r;
//		if (fr2 == 1.)
//			return Color();
//		double T21 = 1. - fr2;
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
//		double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
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
//double LayeredBSDF::pdf(const BSDFSamplingInfos & infos)
//{
//	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	Vector3d wh = (infos.wi + infos.wo).normalized();
//
//	double etaI, etaT, relativeEta, cosThetaT;
//	double fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
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
//	//double cosThetaH = DifferentialGeometry::cosTheta(wh);
//
//	//double Jh = 1. / (4 * std::abs(wh.dot(infos.wo)));
//	//double D = myDistribution.D(wh, myAlphaTop);
//	double pdfDielectric = myTopBSDF->pdf(infos);//std::abs(D * cosThetaH * Jh);
//
//	refractedWi = -refractedWi;
//	refractedWo = -refractedWo;
//
//	//
//	BSDFSamplingInfos refractedInfos = infos;//(refractedWi, refractedWo);
//	refractedInfos.wi = refractedWi;
//	refractedInfos.wo = refractedWo;
//
//	double pdfConductor = myBaseBSDF->pdf(refractedInfos);
//
//	double weight = 0.5;
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
//	myEtaExt = params.getDouble("etaExt", 1.000277); //incident
//	myEtaInt = params.getDouble("etaInt", 1.5046); //transmitted
//	//myTopBSDF = params.getBSDF("topBSDF", RoughDielectric(Parameters()));
//	//myBaseBSDF = params.getBSDF("baseBSDF", RoughDielectric(Parameters()));
//	myAlphaTop = params.getDouble("alphaTop", 0.01);
//	myAlphaBase = params.getDouble("alphaBase", 0.01);
//
//	//myEta = Color(3.20, 2.78, 2.13);
//	//myAbsorption = Color(4.26, 4.19, 3.92);
//
//	myEta = Color(1.18, 0.96, 0.66);
//	myAbsorption = Color(7.04, 6.38, 5.46);
//
//	//Parameters p;
//	//p.addDouble("etaExt", myEtaI);
//	//p.addDouble("etaInt", myEtaT);
//	////p.addTexture("transmittedTexture", Texture::ptr(new ConstantTexture(Color(0.7, 0.1, 0.1))));
//	//p.addDouble("alpha", 0.05);
//	//dielectric = RoughDielectric(p);
//
//	//p.addColor("eta", Color(0.153, 0.144, 0.136));
//	//p.addColor("absorption", Color(3.64, 3.18, 2.50));
//	//p.addDouble("alpha", myAlpha);
//	//conductor = RoughConductor(p);
//}
//
//LayeredBSDF::~LayeredBSDF()
//{
//}
//
////=============================================================================
/////////////////////////////////////////////////////////////////////////////////
//Color LayeredBSDF::fresnel(double etaExt, double etaInt, double cosThetaI, double& etaI, double& etaT, double& relativeEta, double& cosThetaT)
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
//	double sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosThetaI * cosThetaI));
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
//	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
//
//	//Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
//	Vector3d wh = (infos.wi + infos.wo).normalized();
//	double etaI, etaT, relativeEta, cosThetaT;
//	double fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
//	double T12 = 1. - fr1;
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
//	double T21 = 1. - fresnel(myEtaExt, myEtaInt, (-refractedWo).dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
//	Color fr2 = fresnel::fresnelConductor(myEta, myAbsorption, DifferentialGeometry::cosTheta(refractedWi));
//	BSDFSamplingInfos refractedInfos(refractedWi, refractedWo);
//	alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaBase;
//	Color f2 = evalReflection(refractedInfos, fr2, alpha);
//
//	double wiPDotNormal = refractedWi.z();
//	double woPDotNormal = refractedWo.z();
//
//	Color a(std::exp(-myAbsorbance.r * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//		std::exp(-myAbsorbance.g * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
//		std::exp(-myAbsorbance.b * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))));
//
//	double G = myDistribution.G(refractedWi, refractedWo, wh, alpha);
//
//	double t = (1. - G) + T21 * G;
//	return f1 + T12 * f2 * a * t;
//
//
//
//
//	//double cosI = DifferentialGeometry::cosTheta(infos.wi);
//	//double cosO = DifferentialGeometry::cosTheta(infos.wo);
//	//double etaI = myEtaI;
//	//double etaT = myEtaT;
//	//double fr = 0.;
//
//	//double relativeEta = etaI / etaT;
//	////Descartes's law
//	//double sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosI * cosI));
//	////Trigo law
//	//double cosT = std::sqrt(std::max(0., 1 - sinThetaT * sinThetaT));
//	//double t12 = 1. - fresnel::fresnelDielectric(myEtaI, myEtaT, cosI, cosT);
//
//	//Color absorbance(0.7, 0.2, 0.3);
//	//Color a(std::exp(-absorbance.r * myThickness * (1. / std::abs(cosI) + 1. / std::abs(cosO))),
//	//		std::exp(-absorbance.g * myThickness * (1. / std::abs(cosI) + 1. / std::abs(cosO))),
//	//		std::exp(-absorbance.b * myThickness * (1. / std::abs(cosI) + 1. / std::abs(cosO))));
//	//Vector3d wh = (infos.wi + infos.wo).normalized();
//	//double G = shadowingTerm(infos.wi, infos.wo, wh);
//	//double t = (1. - G) + t12 * G;
//	//return dielectric.eval(infos) + t12 * conductor.eval(infos) * a * t;
//}
//
//
//
//Color LayeredBSDF::evalReflection(const BSDFSamplingInfos & infos, const Color& fr, double alpha)
//{
//	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
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
//	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlphaTop;
//
//	Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
//
//	double etaI, etaT, relativeEta, cosThetaT;
//	double fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(normal), etaI, etaT, relativeEta, cosThetaT).r;
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
//		double fr1b = fresnel(myEtaExt, myEtaInt, infos.wo.dot(normal), etaI, etaT, relativeEta, cosThetaT).r;
//		if (fr1b == 1.)
//		{
//			infos.pdf = 0.;
//			return Color();
//		}
//
//		infos.wo = refract(infos.wo, normal, relativeEta, cosThetaT);
//		infos.wo.normalize();
//		double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
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
//	//double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//	//double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
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
//	//double cosO = DifferentialGeometry::cosTheta(infos2.wo);
//	//infos = infos2;
//	//infos.pdf = pdf(infos);
//	//if (infos.pdf < 0)
//	//	return Color();
//	//return eval(infos) * cosO / infos.pdf;
//}
//
////=============================================================================
/////////////////////////////////////////////////////////////////////////////////
////double LayeredBSDF::pdfReflection(const BSDFSamplingInfos& infos, alpha)
////{
////	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
////	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
////
////	if (cosThetaI <= 0. || cosThetaO <= 0.)
////		return 0.;
////
////	Vector3d wh = (infos.wi + infos.wo).normalized();// / (infos.wi + infos.wo).squaredNorm();
////	double cosThetaH = DifferentialGeometry::cosTheta(wh);
////
////	double Jh = 1. / (4 * std::abs(wh.dot(infos.wo)));
////	double D = myDistribution.D(wh, alpha);
////	return D * cosThetaH * Jh;
////}
//
//double LayeredBSDF::pdf(const BSDFSamplingInfos & infos)
//{
//	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	Vector3d wh = (infos.wi + infos.wo).normalized();
//
//	double etaI, etaT, relativeEta, cosThetaT;
//	double fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
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
//	double cosThetaH = DifferentialGeometry::cosTheta(wh);
//
//	double Jh = 1. / (4 * std::abs(wh.dot(infos.wo)));
//	double D = myDistribution.D(wh, myAlphaTop);
//	double pdfDielectric = std::abs(D * cosThetaH * Jh);
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
//	double pdfConductor = std::abs(D * cosThetaH * Jh);
//
//	double weight = 0.5;
//	return weight * pdfDielectric + (1 - weight) * pdfConductor;
//
//
//
//
//
//	//double weight = 0.5;
//	//return weight * dielectric.pdf(infos) + (1 - weight) * conductor.pdf(infos);
//}

RT_REGISTER_TYPE(LayeredBSDF, BSDF)