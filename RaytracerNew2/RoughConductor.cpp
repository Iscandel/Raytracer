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

	myAlpha = params.getDouble("alpha", 0.2);

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
double RoughConductor::distributionBeckmann(const Vector3d& wh)
{
	if (DifferentialGeometry::cosTheta(wh) <= 0.)
		return 0.;

	double cosThetaH2 = DifferentialGeometry::cosTheta(wh) * DifferentialGeometry::cosTheta(wh);
	double tanThetaH2 = DifferentialGeometry::sinTheta2(wh) / cosThetaH2;
	double alpha2 = myAlpha * myAlpha;
	return (std::exp(-tanThetaH2 / alpha2)) / (tools::PI * alpha2 * cosThetaH2 * cosThetaH2);
}

double RoughConductor::shadowingTerm(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh)
{
	return shadowingTermG1(wi, wh) * shadowingTermG1(wo, wh);
}

double RoughConductor::shadowingTermG1(const Vector3d& v, const Vector3d& m)
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
Color RoughConductor::eval(const BSDFSamplingInfos & infos)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	if (cosThetaI <= 0. || cosThetaO <= 0.)
		return Color();

	Vector3d wh = (infos.wi + infos.wo).normalized();// / (infos.wi + infos.wo).squaredNorm();

	double D = myDistribution.D(wh, myAlpha);
	double G = myDistribution.G(infos.wi, infos.wo, wh, myAlpha);

	Color specularPart = myReflectanceTexture->eval(infos.uv) * D * //distributionBeckmann(wh) *
		fresnel::fresnelConductor(myEta, myAbsorption, cosThetaI) *
		G/*shadowingTerm(infos.wi, infos.wo, wh)*/ / (4 * cosThetaI * cosThetaO);

	return specularPart;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color RoughConductor::sample(BSDFSamplingInfos& infos, const Point2d& sample)
{
	if (DifferentialGeometry::cosTheta(infos.wi) <= 0.)
		return Color();

	infos.sampledType = BSDF::GLOSSY_REFLECTION;

	//double theta = std::atan(std::sqrt(- myAlpha * myAlpha * std::log(1 - sample.x())));
	//double phi = 2 * tools::PI * sample.y();

	//Normal3d normal(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta));
	double alpha = (1.2 - 0.2 * std::sqrt(std::abs(DifferentialGeometry::cosTheta(infos.wi)))) * myAlpha;
	Normal3d normal = myDistribution.sampleNormal(sample, alpha);

	//infos.sampledType = BSDF::DELTA_REFLECTION; //to change
	infos.wo = reflect(infos.wi, normal);//2 * infos.wi.dot(normal) * normal - infos.wi;

	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	//if cosTheta < 0, return 0, because pdf = 0...so NaN
	if (cosThetaO <= 0.)
		return Color();

	infos.pdf = pdf(infos);
	if (infos.pdf <= 0)
		return Color();

	return eval(infos)  * std::abs(cosThetaO) / infos.pdf;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double RoughConductor::pdf(const BSDFSamplingInfos& infos)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	if (cosThetaI <= 0. || cosThetaO <= 0.)
		return 0.;

	Vector3d wh = (infos.wi + infos.wo).normalized();// / (infos.wi + infos.wo).squaredNorm();
	double cosThetaH = DifferentialGeometry::cosTheta(wh);

	double Jh = 1. / (4 * std::abs(wh.dot(infos.wo)));
	double D = myDistribution.D(wh, myAlpha);
	return D /*distributionBeckmann(wh)*/ * cosThetaH * Jh;
}

RT_REGISTER_TYPE(RoughConductor, BSDF)