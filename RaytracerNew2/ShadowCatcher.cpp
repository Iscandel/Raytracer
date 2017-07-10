#include "ShadowCatcher.h"

#include "DifferentialGeometry.h"
#include "ObjectFactoryManager.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
ShadowCatcher::ShadowCatcher(const Parameters& params)
{
	myHighThreshold = params.getDouble("highThreshhold", 0.7);
	myRatio = params.getDouble("ratio", 10);
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
		return Color(1.);
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double ShadowCatcher::pdf(const BSDFSamplingInfos& infos)
{
	return 0.;
}

RT_REGISTER_TYPE(ShadowCatcher, BSDF)
