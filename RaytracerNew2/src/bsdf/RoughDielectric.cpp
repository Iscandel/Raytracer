#include "RoughDielectric.h"

#include "texture/ConstantTexture.h"
#include "core/DifferentialGeometry.h"
#include "core/Fresnel.h"
#include "core/Mapping.h"
#include "factory/ObjectFactoryManager.h"
#include "core/Parameters.h"
#include "core/Math.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
RoughDielectric::RoughDielectric(const Parameters& params)
:BSDF(params)
{
	myReflectanceTexture = params.getTexture("reflectanceTexture", Texture::ptr(new ConstantTexture(Color(1.))));
	myTransmittedTexture = params.getTexture("transmittedTexture", Texture::ptr(new ConstantTexture(Color(1.))));

	if (params.hasTexture("alpha"))
		myAlphaTexture = params.getTexture("alpha", nullptr);
	else
		myAlphaTexture = Texture::ptr(new ConstantTexture(params.getReal("alpha", 0.2)));
	//myAlpha = params.getReal("alpha", 0.2f);
	myEtaExt = params.getReal("etaExt", 1.000277f);     /* Interior IOR (default: BK7 borosilicate optical glass) */
	myEtaInt = params.getReal("etaInt", 1.5046f);
	myDistribution = MicrofacetDistribution(params.getString("distribution", "beckmann"));

	if (myEtaExt == myEtaInt)
	{
		ILogger::log() << "RoughDielectric: refractive indices must have different values\n";
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
RoughDielectric::~RoughDielectric()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
//real RoughDielectric::distributionBeckmann(const Vector3d& wh)
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
////=============================================================================
/////////////////////////////////////////////////////////////////////////////////
//real RoughDielectric::shadowingTerm(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh)
//{
//	return shadowingTermG1(wi, wh) * shadowingTermG1(wo, wh);
//}
//
////=============================================================================
/////////////////////////////////////////////////////////////////////////////////
//real RoughDielectric::shadowingTermG1(const Vector3d& v, const Vector3d& m)
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
Color RoughDielectric::eval(const BSDFSamplingInfos & infos)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	bool reflection = cosThetaI * cosThetaO > 0;
	if (reflection)
		return evalReflection(infos);
	else
		return evalRefraction(infos);
}

Color RoughDielectric::evalReflection(const BSDFSamplingInfos & infos, bool adjustAlpha)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	//if (cosThetaI <= 0. || cosThetaO <= 0.)
	//	return Color();

	Vector3d wh = (infos.wi + infos.wo).normalized();
	wh = (real)math::sign(cosThetaI) * wh;

	real alpha;
	if (adjustAlpha)
		alpha = myDistribution.getAdjustedAlpha(myAlphaTexture->eval(infos.uv).average(), cosThetaI);
	else
		alpha = myAlphaTexture->eval(infos.uv).average();

	Color term = myReflectanceTexture->eval(infos.uv) * myDistribution.D(wh, alpha) * 
		//fresnel(myEtaExt, myEtaInt, cosThetaI) *
		fresnel::estimateDielectric(myEtaExt, myEtaInt, infos.wi.dot(wh)) *
		myDistribution.G(infos.wi, infos.wo, wh, alpha) / (4 * cosThetaI * cosThetaO);
	//if (term.r < 0)
	//	return -1 * term;
	return term;
}

Color RoughDielectric::evalRefraction(const BSDFSamplingInfos & infos, bool adjustAlpha)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	real etaI, etaT, relativeEta, cosThetaT;

	
	Color fr = fresnel::estimateDielectric(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT);
	Vector3d wt = halfDirTransmitted(infos.wi, infos.wo, etaI, etaT);
	//fr = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wt), etaI, etaT, relativeEta, cosThetaT);
	//Vector3d wt = (-(etaI * infos.wi + etaT * infos.wo)).normalized();//
	real alpha;
	if (adjustAlpha)
		alpha = myDistribution.getAdjustedAlpha(myAlphaTexture->eval(infos.uv).average(), cosThetaI);
	else
		alpha = myAlphaTexture->eval(infos.uv).average();

	real left =  infos.wi.dot(wt) * infos.wo.dot(wt) / (cosThetaI * cosThetaO);
	Color term = myTransmittedTexture->eval(infos.uv) * left * etaT *etaT * ((-1. * fr) + 1.) *
		 myDistribution.G(infos.wi, infos.wo, wt, alpha) * myDistribution.D(wt, alpha) *relativeEta * relativeEta;
	real denom = (etaI * (infos.wi.dot(wt)) + etaT * infos.wo.dot(wt));
	term /= (denom * denom);
	//if (term.r < 0)
	//	return -1 * term;
	return term;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
//Color RoughDielectric::fresnel(real etaExt, real etaInt, real cosThetaI)
//{
//	real etaI, etaT, relEta, cosT;
//	return fresnel(etaExt, etaInt, cosThetaI, etaI, etaT, relEta, cosT);
//}
//
////=============================================================================
/////////////////////////////////////////////////////////////////////////////////
//Color RoughDielectric::fresnel(real etaExt, real etaInt, real cosThetaI, real& etaI, real& etaT, real& relativeEta, real& cosThetaT)
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
//	real sinThetaT = relativeEta * std::sqrt(std::max((real)0., 1 - cosThetaI * cosThetaI));
//	cosThetaT = std::sqrt(std::max((real)0., 1 - sinThetaT * sinThetaT));
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

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color RoughDielectric::sample(BSDFSamplingInfos& infos, const Point2d& sample)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	
	real alpha = myDistribution.getAdjustedAlpha(myAlphaTexture->eval(infos.uv).average(), cosThetaI);//(1.2f - 0.2f * std::sqrt(std::abs(cosThetaI))) * myAlphaTexture->eval(infos.uv).average();;

	Normal3d normal = myDistribution.sampleNormal(sample, alpha);

	real etaI, etaT, relativeEta, cosThetaT;
	real fr = fresnel::estimateDielectric(myEtaExt, myEtaInt, infos.wi.dot(normal), etaI, etaT, relativeEta, cosThetaT).average();
	//real fr = fresnel::estimateDielectric(myEtaExt, myEtaInt, infos.wi.dot(normal), etaI, etaT, relativeEta, cosThetaT).r();

	//real p = alpha * alpha * sample.x() / (1. - sample.x());
	//real cosThetaM = 1. / std::sqrt(1. + p);
	//real temp = 1 + p / (alpha * alpha);
	//p = tools::INV_PI / (alpha * alpha * cosThetaM * cosThetaM * cosThetaM * temp * temp);
	//infos.pdf = p;

	if (sample.x() <= fr)
	{
		infos.sampledType = BSDF::GLOSSY_REFLECTION;
		infos.wo = reflect(infos.wi, normal);

		real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

		if (DifferentialGeometry::cosTheta(infos.wi) * DifferentialGeometry::cosTheta(infos.wo) <= 0)
			return Color(0.);
			
		infos.pdf = pdfReflection(infos) * fr; //pdfReflection or weighted sum of pdfs ?
		if (infos.pdf <= 0)
			return Color();
		Vector3d wh = (infos.wi + infos.wo).normalized();
		wh = (real)math::sign(cosThetaI) * wh;
		return Color(1.) * infos.wi.dot(normal) * myDistribution.G(infos.wi, infos.wo, wh, alpha) / (cosThetaI * DifferentialGeometry::cosTheta(normal));
		//return Color(1.) * infos.wi.dot(normal) * shadowingTerm(infos.wi, infos.wo, wh) / (cosThetaI * DifferentialGeometry::cosTheta(normal));
		//ILogger::log() << "ALORS : " << evalReflection(infos).r << " " << std::abs(cosThetaO) << " " << pdfReflection(infos) << " " << relativeEta * relativeEta << "\n";
		//return evalReflection(infos)  * std::abs(cosThetaO) / infos.pdf;
	}
	else
	{
		infos.sampledType = BSDF::GLOSSY_TRANSMISSION;	
		infos.wo = refract(infos.wi, normal, relativeEta, cosThetaT);
		//if (!refract(infos.wi, infos.wo, relativeEta, normal))
		//	return Color();
		infos.wo.normalize(); //?

		infos.relativeEta = relativeEta;

		real cosThetaO = infos.wo.dot(normal);
		if (DifferentialGeometry::cosTheta(infos.wi) * DifferentialGeometry::cosTheta(infos.wo) >= 0)
			return Color(0.f);
		
		infos.pdf = pdfRefraction(infos) * (1.f - fr); //pdfRefraction or weighted sum of pdfs ?
		if (infos.pdf <= 0)
			return Color();
		//Vector3d wt = halfDirTransmitted(infos.wi, infos.wo, etaI, etaT);
		//return Color(1.) * infos.wi.dot(normal) * shadowingTerm(infos.wi, infos.wo, wt) / (cosThetaI * DifferentialGeometry::cosTheta(normal));
		//ILogger::log() << "ALORS2 : " << evalRefraction(infos).r << " " << std::abs(cosThetaO) << " " <<  pdfRefraction(infos) << " " <<  relativeEta * relativeEta << "\n";
		//if ((evalRefraction(infos)  * std::abs(cosThetaO) / infos.pdf).isNan())
		//	std::cout << "Stop" << std::endl;
		return evalRefraction(infos)  * std::abs(cosThetaO) / infos.pdf;// *relativeEta * relativeEta;
	}

	
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
real RoughDielectric::pdfRefraction(const BSDFSamplingInfos& infos)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	real etaI, etaT, relativeEta, cosThetaT;
	fresnel::estimateDielectric(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT);

	Vector3d wt = halfDirTransmitted(infos.wi, infos.wo, etaI, etaT);//(etaI < etaT ? -1 : 1) * (etaI * infos.wi + etaT * infos.wo)).normalized();
	real cosThetaWt = DifferentialGeometry::cosTheta(wt);

	real alpha = (1.2f - 0.2f * std::sqrt(std::abs(cosThetaI))) * myAlphaTexture->eval(infos.uv).average();;

	real halfDenom = (etaI * infos.wi.dot(wt) + etaT * infos.wo.dot(wt));
	real Jh = (etaT * etaT * infos.wo.dot(wt)) / (halfDenom * halfDenom);

	return std::abs(myDistribution.D(wt, alpha) /*distributionBeckmann(wt)*/ * cosThetaWt * Jh);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
real RoughDielectric::pdfReflection(const BSDFSamplingInfos& infos)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	//real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	//if (cosThetaI <= 0. || cosThetaO <= 0.)
	//	return 0.;
	real alpha = (1.2f - 0.2f * std::sqrt(std::abs(cosThetaI))) * myAlphaTexture->eval(infos.uv).average();

	Vector3d wh = (infos.wi + infos.wo).normalized();
	wh = (real)math::sign(cosThetaI) * wh;
	real cosThetaH = DifferentialGeometry::cosTheta(wh);

	real Jh = 1.f / (4 * wh.dot(infos.wo));
	return std::abs(myDistribution.D(wh, alpha) /*distributionBeckmann(wh)*/ * cosThetaH * Jh);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
real RoughDielectric::pdf(const BSDFSamplingInfos& infos)
{
	real cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	bool reflection = cosThetaI * cosThetaO > 0;
	
	if (reflection)
	{
		Vector3d wh = (infos.wi + infos.wo).normalized();
		wh = math::sign<real>(cosThetaI) * wh;
		real fr = fresnel::estimateDielectric(myEtaExt, myEtaInt, infos.wi.dot(wh)).average();
		//real fr = fresnel::estimateDielectric(myEtaExt, myEtaInt, infos.wi.dot(wh)).r();
		return pdfReflection(infos) * fr;
	}
	else
	{
		real etaI = myEtaExt, etaT = myEtaInt;
		if (cosThetaI < 0)
			std::swap(etaI, etaT);

		Vector3d wt = halfDirTransmitted(infos.wi, infos.wo, etaI, etaT);
		real fr = fresnel::estimateDielectric(myEtaExt, myEtaInt, infos.wi.dot(wt)).average();
		//real fr = fresnel::estimateDielectric(myEtaExt, myEtaInt, infos.wi.dot(wt)).r();
		return pdfRefraction(infos) * (1.f - fr);
	}
	
//	return fr * pdfReflection(infos) + (1. - fr) * pdfRefraction(infos);
}

RT_REGISTER_TYPE(RoughDielectric, BSDF)
