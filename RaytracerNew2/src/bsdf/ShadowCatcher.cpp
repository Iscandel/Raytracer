#include "ShadowCatcher.h"

#include "core/DifferentialGeometry.h"
#include "factory/ObjectFactoryManager.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
ShadowCatcher::ShadowCatcher(const Parameters& params)
:BSDF(params)
{
	myHighThreshold = params.getReal("highThreshhold", 0.7f);
	myRatio = params.getReal("ratio", 10);
	myShadowRatio = params.getReal("shadowRatio", 1.);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
ShadowCatcher::~ShadowCatcher()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color ShadowCatcher::eval(const BSDFSamplingInfos& infos)
{
	return Color(0.);
	//if (DifferentialGeometry::cosTheta(infos.wi) <= 0. ||
	//	DifferentialGeometry::cosTheta(infos.wo) <= 0.)
	//	return Color();

	//return myAlbedoTexture->eval(infos.uv) * tools::INV_PI;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color ShadowCatcher::sample(BSDFSamplingInfos& infos, const Point2d&)
{
	if (infos.shadowCaught)
	{
		return Color(0.);
	}
	else
	{
		if (infos.ao > myHighThreshold)
			infos.ao = 1.;
		else
			infos.ao /= myRatio;
		infos.wo = -infos.wi;
		infos.pdf = 1.;
		return Color(1.) * myShadowRatio;
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
real ShadowCatcher::pdf(const BSDFSamplingInfos& infos)
{
	return 0.;
}

RT_REGISTER_TYPE(ShadowCatcher, BSDF)
