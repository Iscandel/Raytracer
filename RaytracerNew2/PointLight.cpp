#include "PointLight.h"

#include "ObjectFactoryManager.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
PointLight::PointLight(const Parameters& params)
{
	myPosition = params.getPoint("position", Point3d());
	myIntensity = params.getColor("intensity", Color());
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
PointLight::~PointLight()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
LightSamplingInfos PointLight::sample(const Point3d & pFrom, const Point2d&)
{
	LightSamplingInfos infos;

	infos.interToLight = myPosition - pFrom;
	infos.distance = infos.interToLight.norm();
	double invDist = 1 / infos.distance;
	infos.interToLight *= invDist;
	infos.intensity = myIntensity * invDist * invDist;
	infos.sampledPoint = myPosition;
	infos.pdf = 1.;
	infos.measure = Measure::DISCRETE;

	return infos;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color PointLight::le(const Vector3d&, const Normal3d&) const
{
	return Color();
}

RT_REGISTER_TYPE(PointLight, Light)
