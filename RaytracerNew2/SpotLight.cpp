#include "SpotLight.h"

#include "Math.h"
#include "ObjectFactoryManager.h"
#include "Parameters.h"

SpotLight::SpotLight(const Parameters& params)
{
	myLightToWorld = params.getTransform("toWorld", Transform::ptr(new Transform));
	myIntensity = params.getColor("intensity", Color());
	myCosFalloffStart = std::cos(math::toRadian(params.getReal("falloffStart", 30.)));
	myCosTotalWidth = std::cos(math::toRadian(params.getReal("totalWidth", 60.)));
	myCenter = myLightToWorld->transform(Point3d(0.));
}


SpotLight::~SpotLight()
{
}

LightSamplingInfos SpotLight::sample(const Point3d& pFrom, const Point2d& sample)
{
	LightSamplingInfos infos;

	infos.interToLight = myCenter - pFrom;
	infos.distance = infos.interToLight.norm();
	real invDist = 1 / infos.distance;
	infos.interToLight *= invDist;
	infos.intensity = myIntensity * falloff(-infos.interToLight) * invDist * invDist;
	infos.sampledPoint = myCenter;
	infos.pdf = 1.;
	infos.measure = Measure::DISCRETE;

	return infos;
}

real SpotLight::falloff(const Vector3d& dir)
{
	Vector3d localDir = myLightToWorld->inv().transform(dir);
	real cosTheta = localDir.z();

	if (cosTheta < myCosTotalWidth) return 0.;
	if (cosTheta > myCosFalloffStart) return 1.;

	real delta =( cosTheta - myCosTotalWidth) / (myCosFalloffStart - myCosTotalWidth);

	return (delta * delta) * (delta * delta);
}

real SpotLight::pdf(const Point3d&, const LightSamplingInfos&)
{
	//Point lights cannot be intersected
	return 0.;
}

RT_REGISTER_TYPE(SpotLight, Light)