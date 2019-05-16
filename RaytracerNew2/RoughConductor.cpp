#include "RoughConductor.h"

#include "Color.h"
#include "ConstantTexture.h"
#include "DifferentialGeometry.h"
#include "Fresnel.h"
#include "ObjectFactoryManager.h"
#include "Parameters.h"
#include "Tools.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
RoughConductor::RoughConductor(const Parameters& params)
{
	myReflectanceTexture = params.getTexture("reflectanceTexture", Texture::ptr(new ConstantTexture(Color(1.))));

	if (params.hasTexture("alpha"))
		myAlphaTexture = params.getTexture("alpha", nullptr);
	else
		myAlphaTexture = Texture::ptr(new ConstantTexture(params.getReal("alpha", 0.2)));
	//myAlpha = params.getReal("alpha", 0.2f);

	IORHelper::evalConductorIOR(params, myEta, myAbsorption);

	//Gold
	//myEta = params.getColor("eta", Color(0.22273, 0.42791, 1.2597));
	//myAbsorption = params.getColor("absorption", Color(3.0325, 2.3345, 1.7531));
	myDistribution = MicrofacetDistribution(params.getString("distribution", "beckmann"));
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
RoughConductor::~RoughConductor()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
//real RoughConductor::distributionBeckmann(const Vector3d& wh)
//{
//	if (DifferentialGeometry::cosTheta(wh) <= 0.)
//		return 0.;
//
//	real cosThetaH2 = DifferentialGeometry::cosTheta(wh) * DifferentialGeometry::cosTheta(wh);
//	real tanThetaH2 = DifferentialGeometry::sinTheta2(wh) / cosThetaH2;
//	real alpha2 = myAlpha * myAlpha;
//	return (std::exp(-tanThetaH2 / alpha2)) / (tools::PI * alpha2 * cosThetaH2 * cosThetaH2);
//}
//
//real RoughConductor::shadowingTerm(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh)
//{
//	return shadowingTermG1(wi, wh) * shadowingTermG1(wo, wh);
//}
//
//real RoughConductor::shadowingTermG1(const Vector3d& v, const Vector3d& m)
//{
//	if ((v.dot(m)) / (DifferentialGeometry::cosTheta(v)) <= 0)
//		return 0.f;
//
//	real tanThetaV = DifferentialGeometry::sinTheta(v) /
//		DifferentialGeometry::cosTheta(v);
//	real b = 1.f / (myAlpha * tanThetaV);
//
//	if (b < 1.6f)
//	{
//		return (3.535f * b + 2.181f * b * b) / (1 + 2.276f * b + 2.577f * b * b);
//	}
//	else
//	{
//		return 1.f;
//	}
//}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color RoughConductor::eval(const BSDFSamplingInfos & infos)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	if (cosThetaI <= 0.f || cosThetaO <= 0.f)
		return Color();

	Vector3d wh = (infos.wi + infos.wo).normalized();// / (infos.wi + infos.wo).squaredNorm();

	real D = myDistribution.D(wh, myAlphaTexture->eval(infos.uv).average());
	real G = myDistribution.G(infos.wi, infos.wo, wh, myAlphaTexture->eval(infos.uv).average());

	Color specularPart = myReflectanceTexture->eval(infos.uv) * D * //distributionBeckmann(wh) *
		fresnel::fresnelConductor(myEta, myAbsorption, cosThetaI) *
		G/*shadowingTerm(infos.wi, infos.wo, wh)*/ / (4 * cosThetaI * cosThetaO);

	return specularPart;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color RoughConductor::sample(BSDFSamplingInfos& infos, const Point2d& sample)
{
	if (DifferentialGeometry::cosTheta(infos.wi) <= 0.f)
		return Color();

	infos.sampledType = BSDF::GLOSSY_REFLECTION;

	//real theta = std::atan(std::sqrt(- myAlpha * myAlpha * std::log(1 - sample.x())));
	//real phi = 2 * tools::PI * sample.y();

	//Normal3d normal(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta));
	real alpha = myDistribution.getAdjustedAlpha(myAlphaTexture->eval(infos.uv).average(), DifferentialGeometry::cosTheta(infos.wi));//(1.2f - 0.2f * std::sqrt(std::abs(DifferentialGeometry::cosTheta(infos.wi)))) * myAlphaTexture->eval(infos.uv).average();
	Normal3d normal = myDistribution.sampleNormal(sample, alpha);

	//infos.sampledType = BSDF::DELTA_REFLECTION; //to change
	infos.wo = reflect(infos.wi, normal);//2 * infos.wi.dot(normal) * normal - infos.wi;

	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	//if cosTheta < 0, return 0, because pdf = 0...so NaN
	if (cosThetaO <= 0.f)
		return Color();

	infos.pdf = pdf(infos);
	if (infos.pdf <= 0)
		return Color();

	return eval(infos)  * std::abs(cosThetaO) / infos.pdf;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
real RoughConductor::pdf(const BSDFSamplingInfos& infos)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	if (cosThetaI <= 0. || cosThetaO <= 0.)
		return 0.f;

	Vector3d wh = (infos.wi + infos.wo).normalized();// / (infos.wi + infos.wo).squaredNorm();
	real cosThetaH = DifferentialGeometry::cosTheta(wh);

	real Jh = 1.f / (4 * std::abs(wh.dot(infos.wo)));
	real D = myDistribution.D(wh, myAlphaTexture->eval(infos.uv).average());
	return D /*distributionBeckmann(wh)*/ * cosThetaH * Jh;
}

RT_REGISTER_TYPE(RoughConductor, BSDF)