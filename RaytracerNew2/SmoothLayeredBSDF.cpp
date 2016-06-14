#include "SmoothLayeredBSDF.h"

#include "ObjectFactoryManager.h"

#include "Fresnel.h"
#include "DifferentialGeometry.h"
#include "ConstantTexture.h"

#include <algorithm>

SmoothLayeredBSDF::SmoothLayeredBSDF(const Parameters& params)
	:dielectric(Parameters())
	, conductor(Parameters())
	, myThickness(1.2)
	, myAbsorbance(0.1, 0.7, 0.7)
	, myDistribution("ggx")
{
	myEtaExt = params.getDouble("etaExt", 1.000277); //incident
	myEtaInt = params.getDouble("etaInt", 1.5046); //transmitted
	myAlpha = params.getDouble("etaInt", 0.05);

	myEta = Color(1.18, 0.96, 0.66);
	myAbsorption = Color(7.04, 6.38, 5.46);

	//Parameters p;
	//p.addDouble("etaExt", myEtaI);
	//p.addDouble("etaInt", myEtaT);
	////p.addTexture("transmittedTexture", Texture::ptr(new ConstantTexture(Color(0.7, 0.1, 0.1))));
	//p.addDouble("alpha", 0.05);
	//dielectric = RoughDielectric(p);

	//p.addColor("eta", Color(0.153, 0.144, 0.136));
	//p.addColor("absorption", Color(3.64, 3.18, 2.50));
	//p.addDouble("alpha", myAlpha);
	//conductor = RoughConductor(p);
}

SmoothLayeredBSDF::~SmoothLayeredBSDF()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color SmoothLayeredBSDF::fresnel(double etaExt, double etaInt, double cosThetaI, double& etaI, double& etaT, double& relativeEta, double& cosThetaT)
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

Color SmoothLayeredBSDF::eval(const BSDFSamplingInfos & infos)
{
	return Color(0.);
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlpha;

	//Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
	Vector3d wh = (infos.wi + infos.wo).normalized();
	double etaI, etaT, relativeEta, cosThetaT;
	double fr1 = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
	double T12 = 1. - fr1;


	if (fr1 == 1.)
		return Color();
	Vector3d refractedWi = refract(infos.wi, Normal3d(wh), relativeEta, cosThetaT);
	Vector3d refractedWo = refract(infos.wo, Normal3d(wh), relativeEta, cosThetaT);

	refractedWi.normalize();
	refractedWo.normalize();

	Color f1 = evalReflection(infos, Color(fr1), alpha);

	refractedWi = -refractedWi;
	refractedWo = -refractedWo;

	double T21 = fresnel(myEtaExt, myEtaInt, (-refractedWo).dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
	Color fr2 = fresnel::fresnelConductor(myEta, myAbsorption, DifferentialGeometry::cosTheta(refractedWi));
	BSDFSamplingInfos refractedInfos(refractedWi, refractedWo);
	Color f2 = evalReflection(refractedInfos, fr2, alpha);

	double wiPDotNormal = refractedWi.z();
	double woPDotNormal = refractedWo.z();

	Color a(std::exp(-myAbsorbance.r * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
		std::exp(-myAbsorbance.g * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
		std::exp(-myAbsorbance.b * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))));

	double G = myDistribution.G(refractedWi, refractedWo, wh, alpha);

	double t = (1. - G) + T21 * G;
	return f1 + T12 * f2 * a * t;




	//double cosI = DifferentialGeometry::cosTheta(infos.wi);
	//double cosO = DifferentialGeometry::cosTheta(infos.wo);
	//double etaI = myEtaI;
	//double etaT = myEtaT;
	//double fr = 0.;

	//double relativeEta = etaI / etaT;
	////Descartes's law
	//double sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosI * cosI));
	////Trigo law
	//double cosT = std::sqrt(std::max(0., 1 - sinThetaT * sinThetaT));
	//double t12 = 1. - fresnel::fresnelDielectric(myEtaI, myEtaT, cosI, cosT);

	//Color absorbance(0.7, 0.2, 0.3);
	//Color a(std::exp(-absorbance.r * myThickness * (1. / std::abs(cosI) + 1. / std::abs(cosO))),
	//		std::exp(-absorbance.g * myThickness * (1. / std::abs(cosI) + 1. / std::abs(cosO))),
	//		std::exp(-absorbance.b * myThickness * (1. / std::abs(cosI) + 1. / std::abs(cosO))));
	//Vector3d wh = (infos.wi + infos.wo).normalized();
	//double G = shadowingTerm(infos.wi, infos.wo, wh);
	//double t = (1. - G) + t12 * G;
	//return dielectric.eval(infos) + t12 * conductor.eval(infos) * a * t;
}

Color SmoothLayeredBSDF::eval2(const BSDFSamplingInfos & infos)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlpha;

	//Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
	//Vector3d wh = (infos.wi + infos.wo).normalized();
	double etaI, etaT, relativeEta, cosThetaT;
	double fr1 = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r;
	double T12 = 1. - fr1;


	if (fr1 == 1.)
		return Color();
	Vector3d refractedWi = refract(infos.wi, cosThetaT, relativeEta);
	Vector3d refractedWo = refract(infos.wo, cosThetaT, relativeEta);

	refractedWi.normalize();
	refractedWo.normalize();

	Color f1 = Color(1.) * relativeEta * relativeEta;

	refractedWi = -refractedWi;
	refractedWo = -refractedWo;

	cosThetaI = DifferentialGeometry::cosTheta(-refractedWo);
	double T21 = 1. - fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r;
	Color fr2 = fresnel::fresnelConductor(myEta, myAbsorption, DifferentialGeometry::cosTheta(refractedWi));
	BSDFSamplingInfos refractedInfos(refractedWi, refractedWo);
	Color f2 = fr2;

	double wiPDotNormal = refractedWi.z();
	double woPDotNormal = refractedWo.z();

	Color a(std::exp(-myAbsorbance.r * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
		std::exp(-myAbsorbance.g * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))),
		std::exp(-myAbsorbance.b * myThickness * (1. / std::abs(wiPDotNormal) + 1. / std::abs(woPDotNormal))));

	double G = 1.;//myDistribution.G(refractedWi, refractedWo, wh, alpha);

	double t = (1. - G) + T21 * G;
	return f1 + T12 * f2 * a * t;
}

Color SmoothLayeredBSDF::evalReflection(const BSDFSamplingInfos & infos, const Color& fr, double alpha)
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

Color SmoothLayeredBSDF::sample(BSDFSamplingInfos & infos, const Point2d & sample)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	//double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlpha;
	infos.sampledType = BSDF::DELTA_REFLECTION;
	infos.measure = Measure::DISCRETE;
	//Normal3d normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);

	double etaI, etaT, relativeEta, cosThetaT;
	double fr1 = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r;

	if (fr1 == 1.)
		return Color();
	Vector3d refractedWi = -refract(infos.wi, cosThetaT, relativeEta);
	refractedWi.normalize();

	//normal = myDistribution.sampleNormal(Point2d(myRng.random(0., 1.), myRng.random(0., 1.)), alpha);
	infos.wo = -reflect(refractedWi);

	cosThetaI = DifferentialGeometry::cosTheta(infos.wo);

	fr1 = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r;
	if (fr1 == 1.)
	{
		infos.pdf = 0.;
		return Color();
	}

	infos.wo = refract(infos.wo, cosThetaT, relativeEta);
	infos.wo.normalize();
	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
	infos.pdf = 1.;//pdf(infos);

	if (infos.pdf <= 0.)
		return Color();

	return eval2(infos); //*std::abs(cosThetaO) / infos.pdf;

	//Color res = dielectric.sample(infos, sample);
	//if (res.isZero())
	//	return Color();
	//double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	//double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	//bool reflection = cosThetaI * cosThetaO > 0;
	//if (reflection)
	//	return dielectric.eval(infos) * cosThetaO / dielectric.pdf(infos);

	//BSDFSamplingInfos infos2 = infos;
	//infos2.wi = infos.wo;
	//res = conductor.sample(infos2, sample);
	//if (res.isZero())
	//	return Color();

	//infos2.wi = infos2.wo;
	//dielectric.sample(infos2, sample);
	//double cosO = DifferentialGeometry::cosTheta(infos2.wo);
	//infos = infos2;
	//infos.pdf = pdf(infos);
	//if (infos.pdf < 0)
	//	return Color();
	//return eval(infos) * cosO / infos.pdf;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
//double LayeredBSDF::pdfReflection(const BSDFSamplingInfos& infos, alpha)
//{
//	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
//
//	if (cosThetaI <= 0. || cosThetaO <= 0.)
//		return 0.;
//
//	Vector3d wh = (infos.wi + infos.wo).normalized();// / (infos.wi + infos.wo).squaredNorm();
//	double cosThetaH = DifferentialGeometry::cosTheta(wh);
//
//	double Jh = 1. / (4 * std::abs(wh.dot(infos.wo)));
//	double D = myDistribution.D(wh, alpha);
//	return D * cosThetaH * Jh;
//}

double SmoothLayeredBSDF::pdf(const BSDFSamplingInfos & infos)
{
	return 0.;
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
	double D = myDistribution.D(wh, myAlpha);
	double pdfDielectric = std::abs(D * cosThetaH * Jh);

	refractedWi = -refractedWi;
	refractedWo = -refractedWo;

	//
	wh = (refractedWi + refractedWo).normalized();
	cosThetaH = DifferentialGeometry::cosTheta(wh);

	Jh = 1. / (4 * std::abs(wh.dot(refractedWo)));
	D = myDistribution.D(wh, myAlpha);

	double pdfConductor = std::abs(D * cosThetaH * Jh);

	double weight = 0.5;
	return weight * pdfDielectric + (1 - weight) * pdfConductor;





	//double weight = 0.5;
	//return weight * dielectric.pdf(infos) + (1 - weight) * conductor.pdf(infos);
}

RT_REGISTER_TYPE(SmoothLayeredBSDF, BSDF)