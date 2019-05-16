#include "Mirror.h"

#include "DifferentialGeometry.h"
#include "ObjectFactoryManager.h"
#include "Parameters.h"

Mirror::Mirror(const Parameters&)
{
	//myAlbedo = params.getColor("albedo", Color(0.));
	////myEta = params.getReal("eta", 0.);
	////myAbsorption = params.getReal("absorption", 0.);
	//myEta = params.getColor("eta", Color(0.));
	//myAbsorption = params.getColor("absorption", Color(0.));
}


Mirror::~Mirror()
{
}

Color Mirror::eval(const BSDFSamplingInfos&)
{
	//There are almost no chance that wo is the truely reflected direction
	//of wi. For convenience, we return then 0. As delta function are 
	//handled in a special way, we still get the correct results even if 
	//the given wo is reflect(wi).
	return Color();
}

Color Mirror::sample(BSDFSamplingInfos& infos, const Point2d&)
{
	if (DifferentialGeometry::cosTheta(infos.wi) <= 0.)
		return Color();

	infos.measure = Measure::DISCRETE;
	infos.sampledType = BSDF::DELTA_REFLECTION;
	infos.wo = reflect(infos.wi);
	//No Monte Carlo sampling, there is only one possible direction
	infos.pdf = 1.;

	return Color(1.);
}

real Mirror::pdf(const BSDFSamplingInfos&)
{
	//Discrete BRDF return 0. pdf() is called with light sampling and there 
	//is almost no chance to have generated the direction of shadow ray with 
	//specular brdf
	return 0.;
}

RT_REGISTER_TYPE(Mirror, BSDF)
