#include "SmoothLayeredBSDF.h"

#include "factory/ObjectFactoryManager.h"

#include "bsdf/Diffuse.h"
#include "core/Fresnel.h"
#include "core/DifferentialGeometry.h"
#include "texture/ConstantTexture.h"

#include <algorithm>

SmoothLayeredBSDF::SmoothLayeredBSDF(const Parameters& params)
	: myBaseBSDF()
	, myThickness(1.2f)
{
	myEtaExt = params.getReal("etaExt", 1.000277f); //incident
	myEtaInt = params.getReal("etaInt", 1.5046f); //transmitted
	myThickness = params.getReal("thickness", 1.2f);
	myAbsorbance = params.getColor("absorbance", Color(0.f));
	myBaseBSDF = params.getBSDF("baseBSDF", BSDF::ptr(new Diffuse(Parameters())));

	//real avgAbsorption = std::exp(myAbsorbance.r *(-2 * myThickness)) + std::exp(myAbsorbance.g *(-2 * myThickness)) + std::exp(myAbsorbance.b *(-2 * myThickness));
	//avgAbsorption /= 3.;

	//m_specularSamplingWeight = 1.0f / (avgAbsorption + 1.0f);

}

SmoothLayeredBSDF::~SmoothLayeredBSDF()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color SmoothLayeredBSDF::fresnel(real etaExt, real etaInt, real cosThetaI, real& etaI, real& etaT, real& relativeEta, real& cosThetaT)
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

//Color SmoothLayeredBSDF::eval(const BSDFSamplingInfos & infos)
//{
//	return Color(0.);
//	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//
//	real alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlpha;
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
//	Color f1 = evalReflection(infos, Color(fr1), alpha);
//
//	refractedWi = -refractedWi;
//	refractedWo = -refractedWo;
//
//	real T21 = fresnel(myEtaExt, myEtaInt, (-refractedWo).dot(wh), etaI, etaT, relativeEta, cosThetaT).r;
//	Color fr2 = fresnel::fresnelConductor(myEta, myAbsorption, DifferentialGeometry::cosTheta(refractedWi));
//	BSDFSamplingInfos refractedInfos(refractedWi, refractedWo);
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

Color SmoothLayeredBSDF::eval(const BSDFSamplingInfos & infos)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	//Vector3d wh = (infos.wi + infos.wo).normalized();
	real etaI, etaT, relativeEta, cosThetaT;
	real fr1 = fresnel::estimateDielectric(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).average();
	//real fr1 = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r();
	real T12 = 1.f - fr1;


	if (fr1 == 1.f)
		return Color();
	Vector3d refractedWi = refract(infos.wi, cosThetaT, relativeEta);
	Vector3d refractedWo = refract(infos.wo, cosThetaT, relativeEta);

	refractedWi.normalize();
	refractedWo.normalize();

	Color f1 = Color();//Color(1.) * relativeEta * relativeEta;

	refractedWi = -refractedWi;
	refractedWo = -refractedWo;

	cosThetaI = DifferentialGeometry::cosTheta(-refractedWo);
	real fr2 = fresnel::estimateDielectric(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).average();
	//real fr2 = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r();
	if (fr2 == 1.f)
		return Color();
	real T21 = 1.f - fr2;
	//Color fr2 = fresnel::fresnelConductor(myEta, myAbsorption, DifferentialGeometry::cosTheta(refractedWi));
	BSDFSamplingInfos refractedInfos(refractedWi, refractedWo);
	Color f2 = myBaseBSDF->eval(refractedInfos);//fr2;

	real wiPDotNormal = refractedWi.z();
	real woPDotNormal = refractedWo.z();

	//Color a(std::exp(-myAbsorbance.r * myThickness * (1.f / std::abs(wiPDotNormal) + 1.f / std::abs(woPDotNormal))),
	//	std::exp(-myAbsorbance.g * myThickness * (1.f / std::abs(wiPDotNormal) + 1.f / std::abs(woPDotNormal))),
	//	std::exp(-myAbsorbance.b * myThickness * (1.f / std::abs(wiPDotNormal) + 1.f / std::abs(woPDotNormal))));
	Color a(math::fastExp(-myAbsorbance * myThickness * (1.f / std::abs(wiPDotNormal) + 1.f / std::abs(woPDotNormal))));

	real G = 1.f;//myDistribution.G(refractedWi, refractedWo, wh, alpha);

	real t = (1.f - G) + T21 * G;
	real m_invEta = myEtaExt / myEtaInt;
	
	return f1 + T12 * f2 * a * t * m_invEta * m_invEta * DifferentialGeometry::cosTheta(infos.wo) / DifferentialGeometry::cosTheta(refractedWo);
}

//Color SmoothLayeredBSDF::evalReflection(const BSDFSamplingInfos & infos, const Color& fr, real alpha)
//{
//	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
//
//	Vector3d wh = (infos.wi + infos.wo).normalized();
//	wh = math::sign(cosThetaI) * wh;
//
//	Color term = myDistribution.D(wh, alpha) * fr *
//		myDistribution.G(infos.wi, infos.wo, wh, alpha) / (4 * cosThetaI * cosThetaO);
//	//if (term.r < 0)
//	//	return -1 * term;
//	return term;
//}

Color SmoothLayeredBSDF::sample(BSDFSamplingInfos & infos, const Point2d & sample)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	infos.sampledType = BSDF::GLOSSY_REFLECTION;
	infos.measure = Measure::SOLID_ANGLE;

	real etaI, etaT, relativeEta, cosThetaT;
	real fr1 = fresnel::estimateDielectric(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).average();
	//real fr1 = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r();

	if (fr1 == 1.f)
		return Color();	

	if (sample.x() <= fr1)
	{
		infos.wo = reflect(infos.wi);
		infos.pdf = fr1;
		infos.sampledType = BSDF::DELTA_REFLECTION;
		infos.measure = Measure::DISCRETE;
		return Color(1.f);
	}
	else
	{
		Vector3d refractedWi = -refract(infos.wi, cosThetaT, relativeEta);
		refractedWi.normalize();

		infos.sampledType = BSDF::UNKNOWN;//BSDF::DELTA_REFLECTION;
		//infos.measure = Measure::DISCRETE;

		//
		Color f1 = Color();//Color(1.) * relativeEta * relativeEta;
		real T12 = 1.f - fr1;

		BSDFSamplingInfos refractedInfos(refractedWi);
		Color f2 = myBaseBSDF->sample(refractedInfos, sample);
		if (f2.isZero())
			return Color();

		infos.pdf = (1.f - fr1);// *refractedInfos.pdf;

		if (infos.pdf <= 0.)
			return Color();

		infos.wo = -refractedInfos.wo;

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

		infos.wo = refract(infos.wo, cosThetaT, relativeEta);
		infos.wo.normalize();
		real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

		Color res = (f1 + T12 * f2 * a * T21) *std::abs(cosThetaO) / infos.pdf;
		real m_invEta = myEtaExt / myEtaInt;
		infos.pdf *= refractedInfos.pdf * m_invEta * m_invEta * DifferentialGeometry::cosTheta(infos.wo) / woPDotNormal;
		return res;
		//
	
		//infos.wo = -reflect(refractedWi);

		//cosThetaI = DifferentialGeometry::cosTheta(infos.wo);

		//real fr1b = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r;
		//if (fr1b == 1.)
		//{
		//	infos.pdf = 0.;
		//	return Color();
		//}

		//infos.wo = refract(infos.wo, cosThetaT, relativeEta);
		//infos.wo.normalize();
		//real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
		//infos.pdf = 1. - fr1;//pdf(infos);

		//if (infos.pdf <= 0.)
		//	return Color();

		//return eval2(infos) * std::abs(cosThetaO) / infos.pdf;
	}

	//Color res = dielectric.sample(infos, sample);
	//if (res.isZero())
	//	return Color();
	//real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	//real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

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
	//real cosO = DifferentialGeometry::cosTheta(infos2.wo);
	//infos = infos2;
	//infos.pdf = pdf(infos);
	//if (infos.pdf < 0)
	//	return Color();
	//return eval(infos) * cosO / infos.pdf;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
//real LayeredBSDF::pdfReflection(const BSDFSamplingInfos& infos, alpha)
//{
//	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
//	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
//
//	if (cosThetaI <= 0. || cosThetaO <= 0.)
//		return 0.;
//
//	Vector3d wh = (infos.wi + infos.wo).normalized();// / (infos.wi + infos.wo).squaredNorm();
//	real cosThetaH = DifferentialGeometry::cosTheta(wh);
//
//	real Jh = 1. / (4 * std::abs(wh.dot(infos.wo)));
//	real D = myDistribution.D(wh, alpha);
//	return D * cosThetaH * Jh;
//}

real SmoothLayeredBSDF::pdf(const BSDFSamplingInfos & infos)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	//Vector3d wh = (infos.wi + infos.wo).normalized();

	real etaI, etaT, relativeEta, cosThetaT;
	real fr1 = fresnel::estimateDielectric(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).average();
	//real fr1 = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r();

	if (fr1 == 1.)
		return 0.;
	Vector3d refractedWi = refract(infos.wi, cosThetaT, relativeEta);
	Vector3d refractedWo = refract(infos.wo, cosThetaT, relativeEta);

	refractedWi.normalize();
	refractedWo.normalize();

	//

	//real cosThetaH = DifferentialGeometry::cosTheta(wh);

	//real Jh = 1. / (4 * std::abs(wh.dot(infos.wo)));
	//real D = myDistribution.D(wh, myAlpha);
	real pdfDielectric = 0.;//std::abs(D * cosThetaH * Jh);

	refractedWi = -refractedWi;
	refractedWo = -refractedWo;
	BSDFSamplingInfos refractedInfos = infos;
	refractedInfos.wi = refractedWi;
	refractedInfos.wo = refractedWo;
	real pdfBase = myBaseBSDF->pdf(refractedInfos);
	real weight = 0.f;
	//return weight * pdfDielectric + (1. - weight) * pdfBase * (1. - fr1);
	real m_invEta = myEtaExt / myEtaInt;
	return pdfDielectric + pdfBase * (1.f - fr1) * m_invEta * m_invEta * DifferentialGeometry::cosTheta(infos.wo) / DifferentialGeometry::cosTheta(refractedWo);

	//
	//wh = (refractedWi + refractedWo).normalized();
	//cosThetaH = DifferentialGeometry::cosTheta(wh);

	//Jh = 1. / (4 * std::abs(wh.dot(refractedWo)));
	//D = myDistribution.D(wh, myAlpha);

	//real pdfConductor = std::abs(D * cosThetaH * Jh);

	//real weight = 0.5;
	//return weight * pdfDielectric + (1 - weight) * pdfConductor;





	//real weight = 0.5;
	//return weight * dielectric.pdf(infos) + (1 - weight) * conductor.pdf(infos);
}









//Vector3d SmoothLayeredBSDF::refractIn(const Vector3d &wi, real &R)  {
//	real etaI, etaT, relativeEta, cosThetaT;
//	real cosThetaI = DifferentialGeometry::cosTheta(wi);
//	R = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r;
//	real m_invEta = myEtaExt / myEtaInt;
//	//R = fresnelDielectricExt(std::abs(Frame::cosTheta(wi)), cosThetaT, m_eta);
//
//	//return refract(wi, cosThetaT, relativeEta);
//	return Vector3d(m_invEta * wi.x(), m_invEta * wi.y(), -math::sign(DifferentialGeometry::cosTheta(wi)) * cosThetaT);
//}
//
///// Refract out of the material, preserve sign of direction
//Vector3d SmoothLayeredBSDF::refractOut(const Vector3d &wi, real &R) {
//	real etaI, etaT, relativeEta, cosThetaT;
//	real m_eta = myEtaInt / myEtaExt;
//	real cosThetaI = DifferentialGeometry::cosTheta(wi);
//	R = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r;
//	//R = fresnelDielectricExt(std::abs(Frame::cosTheta(wi)), cosThetaT, m_invEta);
//	return refract(wi, cosThetaT, relativeEta);
//	//return Vector3d(m_eta * wi.x(), m_eta * wi.y(), -math::sign(DifferentialGeometry::cosTheta(wi)) * cosThetaT);
//}
//
//
//
//Color SmoothLayeredBSDF::eval(const BSDFSamplingInfos & infos)
//{
//	real R12, R21;
//	BSDFSamplingInfos bRecInt(infos);
//	bRecInt.wi = refractIn(infos.wi, R12);
//	bRecInt.wo = refractIn(infos.wo, R21);
//
//	if (R12 == 1 || R21 == 1) /* Total internal reflection */
//		return Color(0.0);
//
//	Color result = myBaseBSDF->eval(bRecInt) * (1 - R12) * (1 - R21);
//
//	//if (!sigmaA.isZero())
//
//	Color a(std::exp(-myAbsorbance.r * myThickness * (1. / std::abs(DifferentialGeometry::cosTheta(bRecInt.wi)) + 1. / std::abs(DifferentialGeometry::cosTheta(bRecInt.wo)))),
//		std::exp(-myAbsorbance.g * myThickness * (1. / std::abs(DifferentialGeometry::cosTheta(bRecInt.wi)) + 1. / std::abs(DifferentialGeometry::cosTheta(bRecInt.wo)))),
//		std::exp(-myAbsorbance.b * myThickness * (1. / std::abs(DifferentialGeometry::cosTheta(bRecInt.wi)) + 1. / std::abs(DifferentialGeometry::cosTheta(bRecInt.wo)))));
//
//	result *= a;
//	/* Solid angle compression & irradiance conversion factors */
//	//if (measure == ESolidAngle)
//	real m_invEta = myEtaExt / myEtaInt;
//	result *= m_invEta * m_invEta  /**
//		DifferentialGeometry::cosTheta(infos.wo) *// DifferentialGeometry::cosTheta(bRecInt.wo);
//
//	return result;
//}
//
//Color SmoothLayeredBSDF::sample(BSDFSamplingInfos &bRec, const Point2d &_sample) {
//	real R12;
//	Vector3d wiPrime = refractIn(bRec.wi, R12);
//
//	/* Reallocate samples */
//	real probSpecular = (R12*m_specularSamplingWeight) /
//		(R12*m_specularSamplingWeight +
//		(1 - R12) * (1 - m_specularSamplingWeight));
//
//	bool choseSpecular = true;;
//
//	Point2d sample(_sample);
//		if (sample.x() < probSpecular) {
//			sample.x() /= probSpecular;
//		}
//		else {
//			sample.x() = (sample.x() - probSpecular) / (1 - probSpecular);
//			choseSpecular = false;
//		}
//
//	if (choseSpecular) {
//		bRec.sampledType = BSDF::DELTA_REFLECTION;
//		bRec.wo = reflect(bRec.wi);
//		bRec.relativeEta = 1.0f;
//		bRec.pdf = probSpecular;
//		return Color(1.) * R12 / bRec.pdf;
//	}
//	else {
//		if (R12 == 1.0f) /* Total internal reflection */
//			return Color(0.0);
//
//		Vector3d wiBackup = bRec.wi;
//		bRec.wi = wiPrime;
//		Color result = myBaseBSDF->sample(bRec, sample);
//		bRec.wi = wiBackup;
//		if (result.isZero())
//			return Color(0.0f);
//
//		Vector3d woPrime = bRec.wo;
//
//		Color a(std::exp(-myAbsorbance.r * myThickness * (1. / std::abs(DifferentialGeometry::cosTheta(wiPrime)) + 1. / std::abs(DifferentialGeometry::cosTheta(woPrime)))),
//			std::exp(-myAbsorbance.g * myThickness * (1. / std::abs(DifferentialGeometry::cosTheta(wiPrime)) + 1. / std::abs(DifferentialGeometry::cosTheta(woPrime)))),
//			std::exp(-myAbsorbance.b * myThickness * (1. / std::abs(DifferentialGeometry::cosTheta(wiPrime)) + 1. / std::abs(DifferentialGeometry::cosTheta(woPrime)))));
//
//		result *= a;
//		real R21;
//		bRec.wo = refractOut(woPrime, R21);
//		if (R21 == 1.0f) /* Total internal reflection */
//			return Color(0.0);
//
//		bRec.pdf *= 1.0f - probSpecular;
//		result /= 1.0f - probSpecular;
//
//		result *= (1 - R12) * (1 - R21);
//
//		real m_invEta = myEtaExt / myEtaInt;
//		/* Solid angle compression & irradiance conversion factors */
//		bRec.pdf *= m_invEta * m_invEta * DifferentialGeometry::cosTheta(bRec.wo) / DifferentialGeometry::cosTheta(woPrime);
//
//		return result;
//	}
//}
//
//
//
//real SmoothLayeredBSDF::pdf(const BSDFSamplingInfos &bRec)  {
//	real R12;
//	Vector3d wiPrime = refractIn(bRec.wi, R12);
//
//	/* Reallocate samples */
//	real probSpecular = (R12*m_specularSamplingWeight) /
//		(R12*m_specularSamplingWeight +
//		(1 - R12) * (1 - m_specularSamplingWeight));
//
//		real R21;
//		BSDFSamplingInfos bRecInt(bRec);
//		bRecInt.wi = wiPrime;
//		bRecInt.wo = refractIn(bRec.wo, R21);
//
//		if (R12 == 1 || R21 == 1) /* Total internal reflection */
//			return 0.0f;
//
//		real pdf = myBaseBSDF->pdf(bRecInt);
//		real m_invEta = myEtaExt / myEtaInt;
//		pdf *= m_invEta * m_invEta * DifferentialGeometry::cosTheta(bRec.wo)
//			/ DifferentialGeometry::cosTheta(bRecInt.wo);
//
//		return (pdf * (1 - probSpecular));
//	}

RT_REGISTER_TYPE(SmoothLayeredBSDF, BSDF)