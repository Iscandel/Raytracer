#include "AmbientOcclusion.h"

#include "Intersection.h"
#include "Mapping.h"
#include "ObjectFactoryManager.h"
#include "Scene.h"

AmbientOcclusion::AmbientOcclusion(const Parameters& params)
{
	myRayLength = params.getDouble("length", 10);
}


AmbientOcclusion::~AmbientOcclusion()
{
}

Color AmbientOcclusion::li(Scene& scene, Sampler::ptr sampler, const Ray& ray)
{
	Intersection inter;
	if (!scene.computeIntersection(ray, inter))
		return Color(1., 1., 1.);

	//Point3d interPoint = ray.myOrigin + inter.t * ray.direction();
	Vector3d newDirection = Mapping::uniformSampleHemisphere(sampler, inter.myShadingGeometry.myN);
	Ray newRay(inter.myPoint, newDirection, tools::EPSILON, myRayLength);

	if (scene.computeIntersection(newRay, inter, true))
		return Color(0., 0., 0.);

	return Color(1., 1., 1.);
}

RT_REGISTER_TYPE(AmbientOcclusion, Integrator)