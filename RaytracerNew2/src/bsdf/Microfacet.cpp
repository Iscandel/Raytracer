#include "Microfacet.h"

#include "texture/ConstantTexture.h"
#include "core/DifferentialGeometry.h"
#include "core/Fresnel.h"
#include "core/Mapping.h"
#include "core/Math.h"
#include "factory/ObjectFactoryManager.h"
#include "core/Parameters.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Microfacet::Microfacet(const Parameters& params)
{
	if (params.hasTexture("kd"))
		myKdTexture = params.getTexture("kd", nullptr);
	else
		myKdTexture = Texture::ptr(new ConstantTexture(params.getColor("kd", Color(0.f))));

	//myKd = params.getColor("kd", Color());
	myAlpha = params.getReal("alpha", 0.2f);
	myEtaExt = params.getReal("etaExt", 1.000277f);     /* Interior IOR (default: BK7 borosilicate optical glass) */
	myEtaInt = params.getReal("etaInt", 1.5946f);

	myDistribution = MicrofacetDistribution(params.getString("distribution", "ggx"));
	myKs = params.getReal("probabilitySpecular", 1.f);

	//Color kd = myKdTexture->eval(Point2d());
	//real max = std::max(kd.b, std::max(kd.r, kd.g));
	//myKs = 1 - max;

}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Microfacet::~Microfacet()
{
}

real Microfacet::distributionBeckmann(const Vector3d& wh)
{
	if (DifferentialGeometry::cosTheta(wh) <= 0.)
		return 0.;

	real cosThetaH2 = DifferentialGeometry::cosTheta(wh) * DifferentialGeometry::cosTheta(wh);
	real tanThetaH2 = DifferentialGeometry::sinTheta2(wh) / cosThetaH2;
	real alpha2 = myAlpha * myAlpha;
	return (math::fastExp(-tanThetaH2 / alpha2)) / (math::PI * alpha2 * cosThetaH2 * cosThetaH2);
}

real Microfacet::shadowingTerm(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh)
{
	return shadowingTermG1(wi, wh) * shadowingTermG1(wo, wh);
}

real Microfacet::shadowingTermG1(const Vector3d& v, const Vector3d& m)
{
	if ((v.dot(m)) / (DifferentialGeometry::cosTheta(v)) <= 0)
		return 0.f;

	real tanThetaV = DifferentialGeometry::sinTheta(v) /
					   DifferentialGeometry::cosTheta(v);
	real b = 1.f / (myAlpha * tanThetaV);
	
	if (b < 1.6f)
	{
		return (3.535f * b + 2.181f * b * b) / (1 + 2.276f * b + 2.577f * b * b);
	}
	else
	{
		return 1.f;
	}
}
//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color Microfacet::eval(const BSDFSamplingInfos & infos)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	if (cosThetaI <= 0. || cosThetaO <= 0.)
		return Color();

	Vector3d wh = (infos.wi + infos.wo).normalized();// / (infos.wi + infos.wo).squaredNorm();

	Color diffusePart = (1 - myKs) * myKdTexture->eval(infos.uv) * math::INV_PI; //new : (1-ks)
	Color specularPart = myKs * myDistribution.D(wh, myAlpha) /*distributionBeckmann(wh)*/ *
		fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh)) * //cosThetaI) *
		myDistribution.G(infos.wi, infos.wo, wh, myAlpha) /*shadowingTerm(infos.wi, infos.wo, wh) */ / (4 * cosThetaI * cosThetaO);

	return diffusePart + specularPart;
}

Color Microfacet::fresnel(real etaExt, real etaInt, real cosThetaI)
{
	real etaI = etaExt;
	real etaT = etaInt;
	bool isEntering = true;
	Color fr(0.);

	if (cosThetaI <= 0.)
	{
		isEntering = false;
		std::swap(etaI, etaT);
	}
	real relativeEta = etaI / etaT;

	real sinThetaT = relativeEta * std::sqrt(std::max((real)0., 1 - cosThetaI * cosThetaI));
	real cosThetaT = std::sqrt(std::max((real)0., 1 - sinThetaT * sinThetaT));

	if (sinThetaT >= 1.)
	{
		fr = Color(1.);
	}
	else
	{
		//We want "aperture" of angle, so put abs()
		fr = fresnel::fresnelDielectric(Color(etaI), Color(etaT), std::abs(cosThetaI), cosThetaT);
		//if (isEntering)
		//	cosThetaT = -cosThetaT;
	}

	return fr;
}

Color Microfacet::sample(BSDFSamplingInfos& infos, const Point2d& sample)
{
	if (DifferentialGeometry::cosTheta(infos.wi) <= 0.f)
		return Color();

	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	real alpha = (1.2f - 0.2f * std::sqrt(std::abs(cosThetaI))) * myAlpha;
	Normal3d normal = myDistribution.sampleNormal(sample, alpha);

	//real theta = std::atan(std::sqrt(-myAlpha * myAlpha * std::log(1 - sample.x())));
	//real phi = 2 * math::PI * sample.y();

	//Normal3d normal(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta));

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
	

	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	//if cosTheta <= 0, return 0, because pdf = 0...so NaN
	//if (cosThetaO <= 0.)
	//	return Color();

	infos.pdf = pdf(infos);
	if (infos.pdf == 0)
		return Color();

	return eval(infos) * std::abs(cosThetaO) / infos.pdf;
}

real Microfacet::pdf(const BSDFSamplingInfos& infos)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	//if (cosThetaI <= 0. || cosThetaO <= 0.)
	//	return 0.;

	Vector3d wh = (infos.wi + infos.wo).normalized();
	real cosThetaH = DifferentialGeometry::cosTheta(wh);

	real alpha = (1.2f - 0.2f * std::sqrt(std::abs(cosThetaI))) * myAlpha;

	real Jh = 1.f / (4 * wh.dot(infos.wo));
	return myKs * myDistribution.D(wh, alpha) /*distributionBeckmann(wh)*/ * cosThetaH * Jh + (1 - myKs) * cosThetaO * math::INV_PI;
}

RT_REGISTER_TYPE(Microfacet, BSDF)