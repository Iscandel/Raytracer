#include "DirectLighting.h"

#include "BSDF.h"
#include "Intersection.h"
#include "ObjectFactoryManager.h"
#include "Scene.h"
#include "Tools.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
DirectLighting::DirectLighting(const Parameters& params)
{
	myDepth = params.getInt("depth", 5);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
DirectLighting::~DirectLighting()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color DirectLighting::li(Scene& scene, Sampler::ptr sampler, const Ray& ray)
{
	int depth = 0;
	Color radiance;

	Ray _ray = ray;
	
	while (depth < myDepth)
	{
		Intersection intersection;
		if (scene.computeIntersection(_ray, intersection))
		{
			BSDF::ptr bsdf = intersection.myPrimitive->getBSDF();
			//Point3d inter = _ray.myOrigin + intersection.t * _ray.direction();
			const Scene::LightVector& lights = scene.getLights();
			for (unsigned int  i = 0; i < lights.size(); i++)
			{
				LightSamplingInfos lightInfos = lights[i]->sample(intersection.myPoint, sampler->getNextSample2D());
				Vector3d interToLight = lightInfos.sampledPoint - intersection.myPoint;
				Ray shadowRay(intersection.myPoint, lightInfos.interToLight, tools::EPSILON, interToLight.norm() - tools::EPSILON);
				Intersection tmp;
				if (!scene.computeIntersection(shadowRay, tmp, true))
				{
					Vector3d localWi = intersection.toLocal(lightInfos.interToLight);
					Vector3d localWo = intersection.toLocal(-_ray.direction());
					double cosTheta = DifferentialGeometry::cosTheta(localWi);

					BSDFSamplingInfos bsdfInfos(localWi, localWo);
					
					if(bsdf)
						radiance += lightInfos.intensity * bsdf->eval(bsdfInfos) * cosTheta;
				}
			}

			//_ray = Ray();
			if (bsdf)
			{
				Vector3d localDir = intersection.toLocal(_ray.direction());
				//Vector3d reflected = _ray.direction() - 2 * intersection.myShadingGeometry.myN.dot(_ray.direction()) * intersection.myShadingGeometry.myN;
				Vector3d reflected = Vector3d(-localDir.x(), -localDir.y(), localDir.z());
				_ray = Ray(intersection.myPoint,-intersection.toWorld(reflected));
				//std::cout << "Depth " << depth << " " << ray.myOrigin << " " << ray.direction()  << " its " << intersection.myPoint << " " << reflected << " " << radiance.r << " " << radiance.g << radiance.b << std::endl;
			}
			else
			{
				break;
			}

			depth++;
		}
		else
		{
			break;
		}
			
		
	}

	return radiance;
}

RT_REGISTER_TYPE(DirectLighting, Integrator)