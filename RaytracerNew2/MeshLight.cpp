#include "MeshLight.h"

#include "ISampledShape.h"
#include "Math.h"
#include "ObjectFactoryManager.h"
#include "Parameters.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
MeshLight::MeshLight(const Parameters& params)
:AreaLight(params)
{
	//myRadiance = params.getColor("radiance", Color());
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
MeshLight::~MeshLight()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color MeshLight::power() const
{
	std::shared_ptr<ISampledShape> shape = myShape.lock();
	return math::PI * myRadiance * shape->surfaceArea();
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
LightSamplingInfos MeshLight::sample(const Point3d & pFrom, const Point2d& sample)
{
	LightSamplingInfos infos;

	std::shared_ptr<ISampledShape> shape = myShape.lock();
	shape->sample(sample, infos.sampledPoint, infos.normal);

	infos.interToLight = infos.sampledPoint - pFrom;
	infos.distance = infos.interToLight.norm();
	infos.interToLight /= infos.distance;
	real cosine = infos.normal.dot(-infos.interToLight);
	if (cosine <= 0)
	{
		infos.intensity = Color();
		infos.pdf = 0.;
	}
	else
	{
		infos.intensity = myRadiance;

		//Convert area pdf to pdf wrt solid angle (solid angle subtented by the surface)
		//dw = cos theta * dA / r2
		//and pdf = 1 / dw
		//   _____
		//	|____ | <- surface
		//	\     /
		//	 \   /
		//	  \ /
		infos.pdf = infos.distance * infos.distance / (shape->surfaceArea() * cosine);
	}

	return infos;
}

RT_REGISTER_TYPE(MeshLight, Light)