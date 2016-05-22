#include "LayeredBSDF.h"

#include "ObjectFactoryManager.h"

#include "Fresnel.h"
#include "DifferentialGeometry.h"
#include "ConstantTexture.h"

#include <algorithm>

LayeredBSDF::LayeredBSDF(const Parameters& params)
	:dielectric(Parameters())
	,conductor(Parameters())
	,myThickness(0.2)
{
	myEtaI = params.getDouble("etaExt", 1.000277); //incident
	myEtaT = params.getDouble("etaInt", 1.5046); //transmitted
	myAlpha = params.getDouble("etaInt", 0.05); 

	Parameters p;
	p.addDouble("etaExt", myEtaI);
	p.addDouble("etaInt", myEtaT);
	//p.addTexture("transmittedTexture", Texture::ptr(new ConstantTexture(Color(0.7, 0.1, 0.1))));
	p.addDouble("alpha", 0.05);
	dielectric = RoughDielectric(p);

	p.addColor("eta", Color(0.153, 0.144, 0.136));
	p.addColor("absorption", Color(3.64, 3.18, 2.50));
	p.addDouble("alpha", myAlpha);
	conductor = RoughConductor(p);
}


LayeredBSDF::~LayeredBSDF()
{
}

Color LayeredBSDF::eval(const BSDFSamplingInfos & infos)
{
	double cosI = DifferentialGeometry::cosTheta(infos.wi);
	double cosO = DifferentialGeometry::cosTheta(infos.wo);
	double etaI = myEtaI;
	double etaT = myEtaT;
	double fr = 0.;

	double relativeEta = etaI / etaT;
	//Descartes's law
	double sinThetaT = relativeEta * std::sqrt(std::max(0., 1 - cosI * cosI));
	//Trigo law
	double cosT = std::sqrt(std::max(0., 1 - sinThetaT * sinThetaT));
	double t12 = 1. - fresnel::fresnelDielectric(myEtaI, myEtaT, cosI, cosT);
	Color absorbance(0.7, 0.2, 0.3);
	Color a(std::exp(-absorbance.r * myThickness * (1. / std::abs(cosI) + 1. / std::abs(cosO))),
			std::exp(-absorbance.g * myThickness * (1. / std::abs(cosI) + 1. / std::abs(cosO))),
			std::exp(-absorbance.b * myThickness * (1. / std::abs(cosI) + 1. / std::abs(cosO))));
	Vector3d wh = (infos.wi + infos.wo).normalized();
	double G = shadowingTerm(infos.wi, infos.wo, wh);
	double t = (1. - G) + t12 * G;
	return dielectric.eval(infos) + t12 * conductor.eval(infos) * a * t;
}

Color LayeredBSDF::sample(BSDFSamplingInfos & infos, const Point2d & sample)
{
	Color res = dielectric.sample(infos, sample);
	if (res.isZero())
		return Color();
	double cosThetaI = DifferentialGeometry::cosTheta(infos.wi);
	double cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

	bool reflection = cosThetaI * cosThetaO > 0;
	if (reflection)
		return dielectric.eval(infos) * cosThetaO / dielectric.pdf(infos);

	BSDFSamplingInfos infos2 = infos;
	infos2.wi = infos.wo;
	res = conductor.sample(infos2, sample);
	if (res.isZero())
		return Color();

	infos2.wi = infos2.wo;
	dielectric.sample(infos2, sample);
	double cosO = DifferentialGeometry::cosTheta(infos2.wo);
	infos = infos2;
	infos.pdf = pdf(infos);
	if (infos.pdf < 0)
		return Color();
	return eval(infos) * cosO / infos.pdf;
}

double LayeredBSDF::pdf(const BSDFSamplingInfos & infos)
{
	double weight = 0.5;
	return weight * dielectric.pdf(infos) + (1 - weight) * conductor.pdf(infos);
}

double LayeredBSDF::shadowingTerm(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh)
{
	return shadowingTermG1(wi, wh) * shadowingTermG1(wo, wh);
}

double LayeredBSDF::shadowingTermG1(const Vector3d& v, const Vector3d& m)
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

RT_REGISTER_TYPE(LayeredBSDF, BSDF)