#include "DirectLightingMIS.h"

#include "bsdf/BSDF.h"
#include "core/Intersection.h"
#include "core/Math.h"
#include "factory/ObjectFactoryManager.h"
#include "core/Scene.h"
#include "tools/Tools.h"

DirectLightingMIS::DirectLightingMIS(const Parameters& params)
{
	myDepth = params.getInt("depth", 5);
	mySampleNumber = params.getInt("sampleNumber", 1);
}


DirectLightingMIS::~DirectLightingMIS()
{
}

Color DirectLightingMIS::li(Scene & scene, Sampler::ptr sampler, const Ray & ray, RadianceType::ERadianceType)
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
				//if (!myIsBSDFSampling)
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
						//Vector3d interToLight = lightInfos.sampledPoint - intersection.myPoint;
						Ray shadowRay(intersection.myPoint, lightInfos.interToLight, math::EPSILON, lightInfos.distance/*interToLight.norm()*/ - math::EPSILON);
						Intersection tmp;

						//If the light point is visible from the the inter point
						if (!scene.computeIntersection(shadowRay, tmp, true))
						{
							Vector3d localWi = intersection.toLocal(lightInfos.interToLight);
							Vector3d localWo = intersection.toLocal(-_ray.direction());
							real cosTheta = DifferentialGeometry::cosTheta(localWi);

							BSDFSamplingInfos bsdfInfos(intersection, localWi, localWo);
							bsdfInfos.uv = intersection.myUv; //

							//Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))
							//Add the MIS heuristic
							real bsdfPDF = lights[i]->isDelta() ? 0.f : bsdf->pdf(bsdfInfos);
							real weight = (1.f / mySampleNumber) * powerHeuristic(0.5f, lightInfos.pdf, 0.5f, bsdfPDF);//0.5, bsdf->pdf(bsdfInfos));
							if (std::isnan(weight))
							{
								weight = 0.5f;
								std::cout << "nan value " << lightInfos.pdf << " " << bsdf->pdf(bsdfInfos) << std::endl;
							}
								
		
							if (bsdf)
								radiance += (lightInfos.intensity * bsdf->eval(bsdfInfos) * cosTheta * weight) / (lightInfos.pdf);
						}
					}
				}
				//else //BSDF sampling strategy
				{
					if (!bsdf) //check
						break;
					Vector3d localWi = intersection.toLocal(-_ray.direction());
					BSDFSamplingInfos bsdfInfos(localWi);
					bsdfInfos.uv = intersection.myUv; //
					Color bsdfValue = bsdf->sample(bsdfInfos, sampler->getNextSample2D()); //To change sampler
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
							LightSamplingInfos lightInfos;
							lightInfos.normal = toLightInter.myShadingGeometry.myN;
							lightInfos.sampledPoint = toLightInter.myPoint; //and wi ??????????
							Light::ptr light = toLightInter.myPrimitive->getLight();
							real pdfLight = (bsdfInfos.sampledType & BSDF::DELTA) ? 0.f : light->pdf(intersection.myPoint, lightInfos);
							real weight = (1.f / mySampleNumber)  * powerHeuristic(0.5f, bsdfInfos.pdf, 0.5f, pdfLight);

							Color radianceLight = toLightInter.myPrimitive->le(-shadowRay.direction(), toLightInter.myShadingGeometry.myN);
							radiance += radianceLight * bsdfValue * weight;
						}
					}
					else
					{
						LightSamplingInfos lightInfos;
						lightInfos.interToLight = shadowRay.direction(); //check
						Light::ptr envLight = scene.getEnvironmentLight();
						if (envLight != nullptr)
						{
							Color lightValue = envLight->le(shadowRay.direction());
							real pdfLight = (bsdfInfos.sampledType & BSDF::DELTA) ? 0.f : envLight->pdf(intersection.myPoint, lightInfos);
							real weight = (1.f / mySampleNumber)  * powerHeuristic(0.5f, bsdfInfos.pdf, 0.5f, pdfLight);
							radiance += lightValue * bsdfValue * weight;
						}		
					}

					////new
					//if (bsdfInfos.sampledType == BSDF::DELTA_REFLECTION || bsdfInfos.sampledType == BSDF::DELTA_TRANSMISSION)
					//{
					//	//Vector3d localDir = intersection.toLocal(_ray.direction());
					//	//Vector3d reflected = _ray.direction() - 2 * intersection.myShadingGeometry.myN.dot(_ray.direction()) * intersection.myShadingGeometry.myN;
					//	//in local coords, N = (0,0,1) -> Reflected = 2 N (N.D) - D = (0,0,2dz) - (dx, dy, dz) = (-dx, -dy, dz) ?
					//	//Vector3d reflected = Vector3d(-localDir.x(), -localDir.y(), localDir.z());
					//	_ray = Ray(intersection.myPoint, intersection.toWorld(bsdfInfos.wo));
					//}
					//else
					//{
					//	break;
					//}
					//end new
				}
			}

			BSDFSamplingInfos infos(intersection.toLocal(-_ray.direction()), BSDF::DELTA | BSDF::GLOSSY);
			infos.uv = intersection.myUv; //
			Color test = bsdf->sample(infos, sampler->getNextSample2D()); //To change sampler
			//check if direction agrees with surface normal. If not, color is zero
			if (test.isZero())
				break;

			if ((infos.sampledType & BSDF::DELTA_REFLECTION) == BSDF::DELTA_REFLECTION ||
				(infos.sampledType & BSDF::DELTA_TRANSMISSION) == BSDF::DELTA_TRANSMISSION || 
				(infos.sampledType & BSDF::GLOSSY_TRANSMISSION) == BSDF::GLOSSY_TRANSMISSION ||
				(infos.sampledType & BSDF::GLOSSY_REFLECTION) == BSDF::GLOSSY_REFLECTION)
			//if (infos.sampledType == BSDF::DELTA_REFLECTION || infos.sampledType == BSDF::DELTA_TRANSMISSION)
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

			depth++;
		}
		else
		{
			Light::ptr envLight = scene.getEnvironmentLight();
			if (envLight != nullptr)
			{
				radiance += envLight->le(_ray.direction());
			}
			break;
		}


	}

	return radiance;
}

RT_REGISTER_TYPE(DirectLightingMIS, Integrator)