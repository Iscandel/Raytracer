#include "DirectLightingIS.h"

#include "bsdf/BSDF.h"
#include "core/Intersection.h"
#include "factory/ObjectFactoryManager.h"
#include "core/Scene.h"
#include "tools/Tools.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
DirectLightingIS::DirectLightingIS(const Parameters& params)
{
	myDepth = params.getInt("depth", 5);
	mySampleNumber = params.getInt("sampleNumber", 1);
	myIsBSDFSampling = params.getBool("bsdfSampling", false);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
DirectLightingIS::~DirectLightingIS()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color DirectLightingIS::li(Scene& scene, Sampler::ptr sampler, const Ray& ray, RadianceType::ERadianceType)
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

			//If we have intersected a light, add the radiance
			if (intersection.myPrimitive->isLight())
				radiance += intersection.myPrimitive->le(-_ray.direction(), intersection.myShadingGeometry.myN);
		
			for (int k = 0; k < mySampleNumber; k++)
			{
				//Light sampling strategy
				if (!myIsBSDFSampling)
				{
					//For all the lights
					const Scene::LightVector& lights = scene.getLights();
					for (unsigned int i = 0; i < lights.size(); i++)
					{
						//Sample the light
						LightSamplingInfos lightInfos = lights[i]->sample(intersection.myPoint, sampler->getNextSample2D());

						//If there is no luminance associated, pass
						if (lightInfos.intensity.luminance() <= 1e-7)
							continue;

						//Generate a shadow ray from the first inter to the light sampled point
						Vector3d interToLight = lightInfos.sampledPoint - intersection.myPoint;
						Ray shadowRay(intersection.myPoint, lightInfos.interToLight, math::EPSILON, interToLight.norm() - math::EPSILON);
						Intersection tmp;

						//If the light point is visible from the the inter point
						if (!scene.computeIntersection(shadowRay, tmp, true))
						{
							Vector3d localWi = intersection.toLocal(lightInfos.interToLight);
							Vector3d localWo = intersection.toLocal(-_ray.direction());
							real cosTheta = DifferentialGeometry::cosTheta(localWi);

							BSDFSamplingInfos bsdfInfos(localWi, localWo);

							//Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))
							if (bsdf)
								radiance += (lightInfos.intensity * bsdf->eval(bsdfInfos) * cosTheta) / (mySampleNumber * lightInfos.pdf);
						}
					}
				}
				else //BSDF sampling strategy
				{
					if (!bsdf) //check
						continue;
					Vector3d localWi = intersection.toLocal(-_ray.direction());
					BSDFSamplingInfos bsdfInfos(localWi);
					Color bsdfValue = bsdf->sample(bsdfInfos, sampler->getNextSample2D());
					if (bsdfValue.isZero())
						continue;
					Intersection toLightInter;
					//Go to world coord to "orientate" the hemisphere
					Ray shadowRay(intersection.myPoint, intersection.toWorld(bsdfInfos.wo));
					//Ray shadowRay(intersection.myPoint, (bsdfInfos.wo));

					//Trace a ray from the inter point to the bsdf sampled direction
					if (scene.computeIntersection(shadowRay, toLightInter))
					{
						//If we caught a light
						if (toLightInter.myPrimitive->isLight())
						{
							Color radianceLight = toLightInter.myPrimitive->le(-shadowRay.direction(), toLightInter.myShadingGeometry.myN);
							radiance += radianceLight * bsdfValue / (real)mySampleNumber;
						}
					}
					else
					{
						LightSamplingInfos lightInfos;
						Light::ptr envLight = scene.getEnvironmentLight();
						if (envLight != nullptr)
						{
							Color lightValue = envLight->le(shadowRay.direction(), Normal3d());
							radiance += lightValue * bsdfValue / (real)mySampleNumber;
						}
					}
				}
			}

			if (bsdf)
			{
				BSDFSamplingInfos infos(intersection.toLocal(-_ray.direction()), BSDF::DELTA_REFLECTION);
				bsdf->sample(infos, sampler->getNextSample2D()); //To change sampler

				if (infos.sampledType == BSDF::DELTA_REFLECTION || infos.sampledType == BSDF::DELTA_TRANSMISSION)
				{
					//Vector3d localDir = intersection.toLocal(_ray.direction());
					//Vector3d reflected = _ray.direction() - 2 * intersection.myShadingGeometry.myN.dot(_ray.direction()) * intersection.myShadingGeometry.myN;
					//in local coords, N = (0,0,1) -> Reflected = 2 N (N.D) - D = (0,0,2dz) - (dx, dy, dz) = (-dx, -dy, dz) ?
					//Vector3d reflected = Vector3d(-localDir.x(), -localDir.y(), localDir.z());
					_ray = Ray(intersection.myPoint, intersection.toWorld(infos.wo));
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}

			depth++;
		}
		else
		{
			Light::ptr envLight = scene.getEnvironmentLight();
			if (envLight != nullptr)
			{
				radiance += envLight->le(_ray.direction(), Normal3d());
			}
			break;
		}


	}

	return radiance;
}

RT_REGISTER_TYPE(DirectLightingIS, Integrator)