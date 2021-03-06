#include "AmbientOcclusion.h"

#include "core/Intersection.h"
#include "core/Mapping.h"
#include "core/Math.h"
#include "factory/ObjectFactoryManager.h"
#include "core/Scene.h"

AmbientOcclusion::AmbientOcclusion(const Parameters& params)
:Integrator(params)
{
	myRayLength = params.getReal("length", 10);
}


AmbientOcclusion::~AmbientOcclusion()
{
}

Color AmbientOcclusion::li(Scene& scene, Sampler::ptr sampler, const Ray& ray, RadianceType::ERadianceType)
{
	Intersection inter;
	if (!scene.computeIntersection(ray, inter))
		return Color(1.);

	//Point3d interPoint = ray.myOrigin + inter.t * ray.direction();
	Vector3d newDirection = Mapping::uniformSampleHemisphere(sampler, inter.myShadingGeometry.myN);
	Ray newRay(inter.myPoint, newDirection, math::EPSILON, myRayLength);

	if (scene.computeIntersection(newRay, inter, true))
		return Color(0.);

	return Color(1.);
}

RT_REGISTER_TYPE(AmbientOcclusion, Integrator)