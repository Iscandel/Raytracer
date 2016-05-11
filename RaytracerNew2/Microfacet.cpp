#include "Microfacet.h"

#include "ConstantTexture.h"
#include "DifferentialGeometry.h"
#include "Fresnel.h"
#include "Mapping.h"
#include "ObjectFactoryManager.h"
#include "Parameters.h"
#include "Tools.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Microfacet::Microfacet(const Parameters& params)
{
	//if (params.hasTexture("kd"))
	//	myKdTexture = params.getTexture("kd", nullptr);
	//else
	//	myKdTexture = Texture::ptr(new ConstantTexture(params.getColor("kd", Color(0.5))));

	//myKd = params.getColor("kd", Color());
	myAlpha = params.getDouble("alpha", 0.2);
	myEtaExt = params.getDouble("etaExt", 1.000277);     /* Interior IOR (default: BK7 borosilicate optical glass) */
	myEtaInt = params.getDouble("etaInt", 1.5946);

	double max = std::max(myKd.b, std::max(myKd.r, myKd.g));
	myKs = 1 - max;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Microfacet::~Microfacet()
{
}

double Microfacet::distributionBeckmann(const Vector3d& wh)
{
	if (DifferentialGeometry::cosTheta(wh) <= 0.)
		return 0.;

	double cosThetaH2 = DifferentialGeometry::cosTheta(wh) * DifferentialGeometry::cosTheta(wh);
	double tanThetaH2 = DifferentialGeometry::sinTheta2(wh) / cosThetaH2;
	double alpha2 = myAlpha * myAlpha;
	return (std::exp(-tanThetaH2 / alpha2)) / (tools::PI * alpha2 * cosThetaH2 * cosThetaH2);
}

double Microfacet::shadowingTerm(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh)
{
	return shadowingTermG1(wi, wh) * shadowingTermG1(wo, wh);
}

double Microfacet::shadowingTermG1(const Vector3d& v, const Vector3d& m)
{
	if ((v.dot(m)) / (DifferentialGeometry::cosTheta(v)) <= 0)
		return 0.;

	double tanThetaV = DifferentialGeometry::sinTheta(v) /
					   DifferentialGeometry::cosTheta(v);
	double b = 1. / (myAlpha * tanThetaV);
	
	if (b < 1.6)
	{
		return (3.535 * b + 2.181 * b * b) / (1 + 2.276 * b + 2.577 * b * b);
	}
	else
	{
		return 1.;
	}
}
//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color Microfacet::eval(const BSDFSamplingInfos & infos)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	if (cosThetaI <= 0. || cosThetaO <= 0.)
		return Color();

	Vector3d wh = (infos.wi + infos.wo).normalized();// / (infos.wi + infos.wo).squaredNorm();

	Color diffusePart = myKd * tools::INV_PI;
	Color specularPart = myKs * distributionBeckmann(wh) * 
		fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh)) * //cosThetaI) *
		shadowingTerm(infos.wi, infos.wo, wh) / (4 * cosThetaI * cosThetaO);

	return diffusePart + specularPart;
}

Color Microfacet::fresnel(double etaExt, double etaInt, double cosThetaI)
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

	double sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosThetaI * cosThetaI));
	double cosThetaT = std::sqrt(std::max(0., 1 - sinThetaT * sinThetaT));

	if (sinThetaT >= 1.)
	{
		fr = Color(1.);
	}
	else
	{
		//We want "aperture" of angle, so put abs()
		fr = fresnel::fresnelDielectric(Color(etaI), Color(etaT), std::abs(cosThetaI), cosThetaT);
	}

	return fr;
}

Color Microfacet::sample(BSDFSamplingInfos& infos, const Point2d& sample)
{
	if (DifferentialGeometry::cosTheta(infos.wi) <= 0.)
		return Color();

	double theta = std::atan(std::sqrt(-myAlpha * myAlpha * std::log(1 - sample.x())));
	double phi = 2 * tools::PI * sample.y();

	Normal3d normal(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta));

	if (sample.x() <= myKs)
	{
		infos.sampledType = BSDF::GLOSSY_REFLECTION;
		infos.wo = reflect(infos.wi, normal);
	}
	else
	{
		infos.sampledType = BSDF::DIFFUSE;
		infos.wo = Mapping::squareToCosineWeightedHemisphere(sample);
	}
	

	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	//if cosTheta <= 0, return 0, because pdf = 0...so NaN
	if (cosThetaO <= 0.)
		return Color();

	infos.pdf = pdf(infos);
	if (infos.pdf <= 0)
		return Color();

	return eval(infos) * std::abs(cosThetaO) / infos.pdf;
}

double Microfacet::pdf(const BSDFSamplingInfos& infos)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	if (cosThetaI <= 0. || cosThetaO <= 0.)
		return 0.;

	Vector3d wh = (infos.wi + infos.wo).normalized();
	double cosThetaH = DifferentialGeometry::cosTheta(wh);

	double Jh = 1. / (4 * wh.dot(infos.wo));
	return myKs * distributionBeckmann(wh) * cosThetaH * Jh + (1 - myKs) * cosThetaO * tools::INV_PI;
}

RT_REGISTER_TYPE(Microfacet, BSDF)