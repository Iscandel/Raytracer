#include "RoughDielectric.h"

#include "ConstantTexture.h"
#include "DifferentialGeometry.h"
#include "Fresnel.h"
#include "Mapping.h"
#include "ObjectFactoryManager.h"
#include "Parameters.h"
#include "Tools.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
RoughDielectric::RoughDielectric(const Parameters& params)
{
	myReflectanceTexture = params.getTexture("reflectanceTexture", Texture::ptr(new ConstantTexture(Color(1.))));
	myTransmittedTexture = params.getTexture("transmittedTexture", Texture::ptr(new ConstantTexture(Color(1.))));

	myAlpha = params.getDouble("alpha", 0.2);
	myEtaExt = params.getDouble("etaExt", 1.000277);     /* Interior IOR (default: BK7 borosilicate optical glass) */
	myEtaInt = params.getDouble("etaInt", 1.5046);
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
double RoughDielectric::distributionBeckmann(const Vector3d& wh)
{
	if (DifferentialGeometry::cosTheta(wh) <= 0.)
		return 0.;

	double cosThetaH2 = DifferentialGeometry::cosTheta(wh) * DifferentialGeometry::cosTheta(wh);
	double tanThetaH2 = DifferentialGeometry::sinTheta2(wh) / cosThetaH2;
	double alpha2 = myAlpha * myAlpha;
	return (std::exp(-tanThetaH2 / alpha2)) / (tools::PI * alpha2 * cosThetaH2 * cosThetaH2);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double RoughDielectric::shadowingTerm(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh)
{
	return shadowingTermG1(wi, wh) * shadowingTermG1(wo, wh);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double RoughDielectric::shadowingTermG1(const Vector3d& v, const Vector3d& m)
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
Color RoughDielectric::eval(const BSDFSamplingInfos & infos)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	bool reflection = cosThetaI * cosThetaO > 0;
	if (reflection)
		return evalReflection(infos);
	else
		return evalRefraction(infos);
}

Color RoughDielectric::evalReflection(const BSDFSamplingInfos & infos)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	//if (cosThetaI <= 0. || cosThetaO <= 0.)
	//	return Color();

	Vector3d wh = (infos.wi + infos.wo).normalized();
	wh = tools::sign(cosThetaI) * wh;

	Color term = myReflectanceTexture->eval(infos.uv) * myDistribution.D(wh, myAlpha) * 
		//fresnel(myEtaExt, myEtaInt, cosThetaI) *
		fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh)) *
		myDistribution.G(infos.wi, infos.wo, wh, myAlpha) / (4 * cosThetaI * cosThetaO);
	//if (term.r < 0)
	//	return -1 * term;
	return term;
}

Color RoughDielectric::evalRefraction(const BSDFSamplingInfos & infos)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	double etaI, etaT, relativeEta, cosThetaT;

	
	Color fr = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT);
	Vector3d wt = halfDirTransmitted(infos.wi, infos.wo, etaI, etaT);
	//fr = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wt), etaI, etaT, relativeEta, cosThetaT);
	//Vector3d wt = (-(etaI * infos.wi + etaT * infos.wo)).normalized();//
	
	double left =  infos.wi.dot(wt) * infos.wo.dot(wt) / (cosThetaI * cosThetaO);
	Color term = myTransmittedTexture->eval(infos.uv) * left * etaT *etaT * ((-1. * fr) + 1.) *
		 myDistribution.G(infos.wi, infos.wo, wt, myAlpha) * myDistribution.D(wt, myAlpha) *relativeEta * relativeEta;
	double denom = (etaI * (infos.wi.dot(wt)) + etaT * infos.wo.dot(wt));
	term /= (denom * denom);
	//if (term.r < 0)
	//	return -1 * term;
	return term;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color RoughDielectric::fresnel(double etaExt, double etaInt, double cosThetaI)
{
	double etaI, etaT, relEta, cosT;
	return fresnel(etaExt, etaInt, cosThetaI, etaI, etaT, relEta, cosT);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color RoughDielectric::fresnel(double etaExt, double etaInt, double cosThetaI, double& etaI, double& etaT, double& relativeEta, double& cosThetaT)
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

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color RoughDielectric::sample(BSDFSamplingInfos& infos, const Point2d& sample)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlpha;

	Normal3d normal = myDistribution.sampleNormal(sample, alpha);

	double etaI, etaT, relativeEta, cosThetaT;
	double fr = fresnel(myEtaExt, myEtaInt, infos.wi.dot(normal), etaI, etaT, relativeEta, cosThetaT).r;

	//double p = alpha * alpha * sample.x() / (1. - sample.x());
	//double cosThetaM = 1. / std::sqrt(1. + p);
	//double temp = 1 + p / (alpha * alpha);
	//p = tools::INV_PI / (alpha * alpha * cosThetaM * cosThetaM * cosThetaM * temp * temp);
	//infos.pdf = p;

	if (sample.x() <= fr)
	{
		infos.sampledType = BSDF::GLOSSY_REFLECTION;
		infos.wo = reflect(infos.wi, normal);

		double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

		if (DifferentialGeometry::cosTheta(infos.wi) * DifferentialGeometry::cosTheta(infos.wo) <= 0)
			return Color(0.);
			
		infos.pdf = pdfReflection(infos) * fr; //pdfReflection or weighted sum of pdfs ?
		if (infos.pdf <= 0)
			return Color();
		Vector3d wh = (infos.wi + infos.wo).normalized();
		wh = tools::sign(cosThetaI) * wh;
		return Color(1.) * infos.wi.dot(normal) * shadowingTerm(infos.wi, infos.wo, wh) / (cosThetaI * DifferentialGeometry::cosTheta(normal));
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

		double cosThetaO = infos.wo.dot(normal);
		if (DifferentialGeometry::cosTheta(infos.wi) * DifferentialGeometry::cosTheta(infos.wo) >= 0)
			return Color(0.);
		
		infos.pdf = pdfRefraction(infos) * (1. - fr); //pdfRefraction or weighted sum of pdfs ?
		if (infos.pdf <= 0)
			return Color();
		//Vector3d wt = halfDirTransmitted(infos.wi, infos.wo, etaI, etaT);
		//return Color(1.) * infos.wi.dot(normal) * shadowingTerm(infos.wi, infos.wo, wt) / (cosThetaI * DifferentialGeometry::cosTheta(normal));
		//ILogger::log() << "ALORS2 : " << evalRefraction(infos).r << " " << std::abs(cosThetaO) << " " <<  pdfRefraction(infos) << " " <<  relativeEta * relativeEta << "\n";
		return evalRefraction(infos)  * std::abs(cosThetaO) / infos.pdf;// *relativeEta * relativeEta;
	}

	
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double RoughDielectric::pdfRefraction(const BSDFSamplingInfos& infos)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);

	double etaI, etaT, relativeEta, cosThetaT;
	fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT);

	Vector3d wt = halfDirTransmitted(infos.wi, infos.wo, etaI, etaT);//(etaI < etaT ? -1 : 1) * (etaI * infos.wi + etaT * infos.wo)).normalized();
	double cosThetaWt = DifferentialGeometry::cosTheta(wt);

	double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlpha;

	double halfDenom = (etaI * infos.wi.dot(wt) + etaT * infos.wo.dot(wt));
	double Jh = (etaT * etaT * infos.wo.dot(wt)) / (halfDenom * halfDenom);

	return std::abs(myDistribution.D(wt, alpha) /*distributionBeckmann(wt)*/ * cosThetaWt * Jh);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double RoughDielectric::pdfReflection(const BSDFSamplingInfos& infos)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	//double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	//if (cosThetaI <= 0. || cosThetaO <= 0.)
	//	return 0.;
	double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlpha;

	Vector3d wh = (infos.wi + infos.wo).normalized();
	wh = tools::sign(cosThetaI) * wh;
	double cosThetaH = DifferentialGeometry::cosTheta(wh);

	double Jh = 1. / (4 * wh.dot(infos.wo));
	return std::abs(myDistribution.D(wh, alpha) /*distributionBeckmann(wh)*/ * cosThetaH * Jh);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double RoughDielectric::pdf(const BSDFSamplingInfos& infos)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	bool reflection = cosThetaI * cosThetaO > 0;
	
	if (reflection)
	{
		Vector3d wh = (infos.wi + infos.wo).normalized();
		wh = tools::sign(cosThetaI) * wh;
		double fr = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wh)).r;
		return pdfReflection(infos) * fr;
	}
	else
	{
		double etaI = myEtaExt, etaT = myEtaInt;
		if (cosThetaI < 0)
			std::swap(etaI, etaT);

		Vector3d wt = halfDirTransmitted(infos.wi, infos.wo, etaI, etaT);
		double fr = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wt)).r;
		return pdfRefraction(infos) * (1. - fr);
	}
	
//	return fr * pdfReflection(infos) + (1. - fr) * pdfRefraction(infos);
}

RT_REGISTER_TYPE(RoughDielectric, BSDF)
