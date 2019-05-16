#include "AreaLight.h"

#include "ISampledShape.h"
#include "Math.h"
#include "ObjectFactoryManager.h"
#include "Parameters.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
AreaLight::AreaLight(const Parameters& params)
{
	myRadiance = params.getColor("radiance", Color());
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
AreaLight::~AreaLight()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color AreaLight::power() const
{
	std::shared_ptr<ISampledShape> shape = myShape.lock();
	return math::PI * myRadiance * shape->surfaceArea();
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
LightSamplingInfos AreaLight::sample(const Point3d & pFrom, const Point2d& sample)
{
	LightSamplingInfos infos;

	std::shared_ptr<ISampledShape> shape = myShape.lock();
	shape->sample(sample, infos.sampledPoint, infos.normal);

	infos.interToLight = infos.sampledPoint - pFrom;
	infos.distance = infos.interToLight.norm();
	infos.interToLight /= infos.distance;
	real cosine = infos.normal.dot(-infos.interToLight);

	if (cosine < 0)
	{
		infos.intensity = Color();
		infos.pdf = 0.;
	}
	else
	{
		infos.intensity = myRadiance;
		infos.pdf = shape->pdf(pFrom, infos.sampledPoint, infos.normal);
	}

	return infos;
}

real AreaLight::pdf(const Point3d& pFrom, const LightSamplingInfos& infos)
{
	std::shared_ptr<ISampledShape> shape = myShape.lock();
	return shape->pdf(pFrom, infos.sampledPoint, infos.normal);
}

 RT_REGISTER_TYPE(AreaLight, Light)