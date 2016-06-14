#include "Conductor.h"

#include "ConstantTexture.h"
#include "DifferentialGeometry.h"
#include "Fresnel.h"
#include "ObjectFactoryManager.h"
#include "Parameters.h"


//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Conductor::Conductor(const Parameters& params)
{
	myReflectanceTexture = params.getTexture("reflectanceTexture", Texture::ptr(new ConstantTexture(Color(1.))));

	myEta = params.getColor("eta", Color(0.));
	myAbsorption = params.getColor("absorption", Color(0.));
}


//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Conductor::~Conductor()
{
}


//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color Conductor::eval(const BSDFSamplingInfos& infos)
{
	if (DifferentialGeometry::cosTheta(infos.wi) <= 0. ||
		DifferentialGeometry::cosTheta(infos.wo) <= 0. ||
		infos.measure != Measure::DISCRETE)
		return Color();

	double cosI = DifferentialGeometry::cosTheta(infos.wi);

	return myReflectanceTexture->eval(infos.uv) * fresnel::fresnelConductor(myEta, myAbsorption, cosI);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color Conductor::sample(BSDFSamplingInfos& infos, const Point2d&)
{
	if (DifferentialGeometry::cosTheta(infos.wi) <= 0.)
		return Color();

	infos.sampledType = BSDF::DELTA_REFLECTION;
	infos.measure = Measure::DISCRETE;
	infos.wo = reflect(infos.wi);
	infos.pdf = 1.;

	double cosI = DifferentialGeometry::cosTheta(infos.wi);

	//pdf = 1, no need to divide
	//no need to multiply by cos thetaWo : brdf is fresnel / cos theta
	//check pbrt book p 470
	return myReflectanceTexture->eval(infos.uv) * fresnel::fresnelConductor(myEta, myAbsorption, cosI);// *cosWo;
}


//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double Conductor::pdf(const BSDFSamplingInfos& infos) //To add discrete test
{
	if (DifferentialGeometry::cosTheta(infos.wi) <= 0. ||
		DifferentialGeometry::cosTheta(infos.wo) <= 0.)
		return 0.;

	if(infos.measure == Measure::DISCRETE) //check wo corresponds to reflected wi
		return 1.;
	return 0.;
}

RT_REGISTER_TYPE(Conductor, BSDF)