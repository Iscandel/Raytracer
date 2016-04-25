#include "RoughDielectric.h"



#include "Microfacet.h"

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
	myAlpha = params.getDouble("alpha", 0.2);
	myEtaExt = params.getDouble("etaExt", 1.000277);     /* Interior IOR (default: BK7 borosilicate optical glass) */
	myEtaInt = params.getDouble("etaInt", 1.5046);

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

	Vector3d wh = (infos.wi + infos.wo).normalized();// / (infos.wi + infos.wo).squaredNorm();

	Color term =  distributionBeckmann(wh) *
		fresnel(myEtaExt, myEtaInt, cosThetaI) *
		shadowingTerm(infos.wi, infos.wo, wh) / (4 * cosThetaI * cosThetaO);

	return term;
}

Color RoughDielectric::evalRefraction(const BSDFSamplingInfos & infos)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	//if (cosThetaI <= 0. || cosThetaO <= 0.)
	//	return Color();

	double etaI, etaT, relativeEta, cosThetaT;

	Color fr = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT);
	//Color fr = fresnel(myEtaExt, myEtaInt, infos.wi.dot(wt), etaI, etaT, relativeEta, cosThetaT);
	Vector3d wt = (-(etaI * infos.wi + etaT * infos.wo)).normalized();// / (infos.wi + infos.wo).squaredNorm();
	
	double left = infos.wi.dot(wt) * infos.wo.dot(wt) / (cosThetaI * cosThetaO);
	Color term =  left * etaT *etaT * ((-1. * fr) + 1.) *  
		shadowingTerm(infos.wi, infos.wo, wt) * distributionBeckmann(wt) ;
	double denom = (etaI * (infos.wi.dot(wt)) + etaT * infos.wo.dot(wt));
	term /= (denom * denom);

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
	//if (cosThetaI <= 0.)
	//	return Color();

	double alpha = (1.2 - 0.2 * std::sqrt(std::abs(cosThetaI))) * myAlpha;
	double theta = std::atan(std::sqrt(-alpha * alpha * std::log(1 - sample.x())));
	double phi = 2 * tools::PI * sample.y();

	Normal3d normal(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta));

	double etaI, etaT, relativeEta, cosThetaT;
	double fr = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r;

	if (sample.x() <= fr)
	{
		infos.sampledType = BSDF::GLOSSY_REFLECTION;
		infos.wo = reflect(infos.wi, normal);

		double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

		//if cosTheta <= 0, return 0, because pdf = 0...so NaN
		if (cosThetaO <= 0.)
			return Color();

		//if (pdfReflection(infos) == 0.)
		//{
		//	ILogger::log() << "nul\n";
		//	return Color();
		//}
			
		infos.pdf = pdfReflection(infos);
		if (infos.pdf <= 0)
			return Color();

		//ILogger::log() << "ALORS : " << evalReflection(infos).r << " " << std::abs(cosThetaO) << " " << pdfReflection(infos) << " " << relativeEta * relativeEta << "\n";
		return evalReflection(infos)  * std::abs(cosThetaO) / infos.pdf;
	}
	else
	{
		infos.sampledType = BSDF::GLOSSY_TRANSMISSION;
		infos.wo = refract(infos.wi, relativeEta, normal);
		//infos.wo.normalize(); //?

		double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
		//ILogger::log() << "bye\n";
		//if cosTheta <= 0, return 0, because pdf = 0...so NaN
		//if (cosThetaO <= 0.)
		//	return Color();

		//if (pdfRefraction(infos) == 0.)
		//{
		//	ILogger::log() << "nul\n";
		//	return Color();
		//}
		
		infos.pdf = pdfRefraction(infos);
		if (infos.pdf <= 0)
			return Color();

		//ILogger::log() << "ALORS2 : " << evalRefraction(infos).r << " " << std::abs(cosThetaO) << " " <<  pdfRefraction(infos) << " " <<  relativeEta * relativeEta << "\n";
		return evalRefraction(infos)  * std::abs(cosThetaO) / infos.pdf * relativeEta * relativeEta;
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double RoughDielectric::pdfRefraction(const BSDFSamplingInfos& infos)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	//if (cosThetaI <= 0. || cosThetaO <= 0.)
	//	return 0.;

	double etaI, etaT, relativeEta, cosThetaT;
	double fr = fresnel(myEtaExt, myEtaInt, cosThetaI, etaI, etaT, relativeEta, cosThetaT).r;

	Vector3d wt = (-(etaI * infos.wi + etaT * infos.wo)).normalized();// / (infos.wi + infos.wo).squaredNorm();
	double cosThetaWt = DifferentialGeometry::cosTheta(wt);

	double halfDenom = (etaI * infos.wi.dot(wt) + etaT * infos.wo.dot(wt));
	double Jh = (etaT * etaT * infos.wo.dot(wt)) / (halfDenom * halfDenom);

	return std::abs(distributionBeckmann(wt) * cosThetaWt * Jh);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double RoughDielectric::pdfReflection(const BSDFSamplingInfos& infos)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	//if (cosThetaI <= 0. || cosThetaO <= 0.)
	//	return 0.;

	Vector3d wh = (infos.wi + infos.wo).normalized();
	double cosThetaH = DifferentialGeometry::cosTheta(wh);

	double Jh = 1. / (4 * wh.dot(infos.wo));
	return std::abs(distributionBeckmann(wh) * cosThetaH * Jh);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double RoughDielectric::pdf(const BSDFSamplingInfos& infos)
{
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	bool reflection = cosThetaI * cosThetaO > 0;
	double fr = fresnel(myEtaExt, myEtaInt, cosThetaI).r;
	if (reflection)
		return pdfReflection(infos) * fr;
	else
		return pdfRefraction(infos) * (1. - fr);
	
	return fr * pdfReflection(infos) + (1. - fr) * pdfRefraction(infos);
}

RT_REGISTER_TYPE(RoughDielectric, BSDF)
