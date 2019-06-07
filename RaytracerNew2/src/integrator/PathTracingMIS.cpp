#include "PathTracingMIS.h"

#include "core/Parameters.h"
#include "core/Intersection.h"
#include "core/Scene.h"


//MC enables to solve integrals such : I_a^b f(x) dx <-> Sum_n (1 / N) * f(x) / p(x)
//<-> Sum_n ((b-a) / N) * f(x) with pdf = 1 / (b-a), which mean proba is 1 over the range
//In path tracing, we trace 1 ray at each point, so N = 1. In direct lighting, 
//we casted N shadow rays for each inter point.
PathTracingMIS::PathTracingMIS(const Parameters& params)
:Integrator(params)
{
	//std::string sStrategy = params.getString("strategy", lightStrategy::STRING[LightSamplingStrategy::ONE_LIGHT_UNIFORM]);
	//myStrategy = myStrategiesByName[sStrategy];

	myMinDepth = params.getInt("minDepth", 3);
	myMaxDepth = params.getInt("maxDepth", 1000);
}


PathTracingMIS::~PathTracingMIS()
{
}

Color PathTracingMIS::li(Scene & scene, Sampler::ptr sampler, const Ray & _ray, RadianceType::ERadianceType radianceType)
{
	//std::cout << "START" << std::endl;
	Color radiance;
	const real proba = 0.8f;
	Color throughput(1.f);
	real eta = 1.f;
	int depth = 0;
	Ray ray(_ray);
	bool shadowCaught = false;

	Intersection intersection;
	if (!scene.computeIntersection(ray, intersection))
	{
		if (radianceType & RadianceType::EMISSION)
		{
			Light::ptr envLight = scene.getEnvironmentLight();
			if (envLight != nullptr)
			{
				radiance += envLight->le(ray.direction());
			}
		}

		return radiance;
	}
	bool noIntersection = false;

	while (depth < myMaxDepth)
	{
		//if (noIntersection)
		//{
		//	Light::ptr envLight = scene.getEnvironmentLight();
		//	if (envLight != nullptr)
		//	{
		//		radiance += throughput * envLight->le(ray.direction());
		//	}

		//	return radiance;
		//}

		BSDF::ptr bsdf = intersection.myPrimitive->getBSDF();

		if (bsdf == nullptr)
			return radiance;

		//If we have intersected a light, add the radiance
		if (/*depth == 0*/(radianceType & RadianceType::EMISSION) && intersection.myPrimitive->isLight())
			radiance += throughput * intersection.myPrimitive->le(-ray.direction(), intersection.myPoint, intersection.myShadingGeometry.myN);

		if (intersection.myPrimitive->getBSSRDF() && (radianceType & RadianceType::SUBSURFACE_RADIANCE))
			radiance += throughput * intersection.myPrimitive->getBSSRDF()->eval(intersection.myPoint, intersection, -ray.direction());

		//Light sampling strategy
		if (myLightStrategy == LightSamplingStrategy::ALL_LIGHT)
		{
			LightSamplingInfos lightInfos;
			const Scene::LightVector& lights = scene.getLights();
			for (unsigned int i = 0; i < lights.size(); i++)
			{
				Color value = sampleOneLight(lights[i], intersection.myPoint, sampler->getNextSample2D(), scene, lightInfos);
				if (!value.isZero())
				{

					Vector3d localWi = intersection.toLocal(lightInfos.interToLight);
					Vector3d localWo = intersection.toLocal(-ray.direction());
					real cosTheta = DifferentialGeometry::cosTheta(localWi);

					BSDFSamplingInfos bsdfInfos(intersection, localWi, localWo);
					bsdfInfos.uv = intersection.myUv; //

					//Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))

					//Same as specular BSDF : delta lights cannot be intersected during 
					//BSDF sampling. Then, MIS is not applyable and the "weight" is 1
					real bsdfPDF = lights[i]->isDelta() ? 0.f : bsdf->pdf(bsdfInfos);
					//Add the MIS heuristic
					real weight = powerHeuristic(lightInfos.pdf, bsdfPDF);

					radiance += throughput * value * bsdf->eval(bsdfInfos) * cosTheta * weight;

					if (DifferentialGeometry::cosTheta(localWi) <= 0.)
						shadowCaught = false; //true
					else
						shadowCaught = false;
				}
				else
				{
					shadowCaught = true;
				}
			}
		}
		else
		{
			LightSamplingInfos lightInfos;
			Color value = sampleLightDirect(intersection.myPoint, sampler->getNextSample2D(), scene, lightInfos, myLightStrategy);
			if (!value.isZero())
			{
				Vector3d localWi = intersection.toLocal(lightInfos.interToLight);
				Vector3d localWo = intersection.toLocal(-ray.direction());
				real cosTheta = DifferentialGeometry::cosTheta(localWi);

				BSDFSamplingInfos bsdfInfos(intersection, localWi, localWo);
				bsdfInfos.uv = intersection.myUv;

				//Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))

				//Same as specular BSDF : delta lights cannot be intersected during 
				//BSDF sampling. Then, MIS is not applyable and the "weight" is 1
				real bsdfPDF = lightInfos.light->isDelta() ? 0.f : bsdf->pdf(bsdfInfos);
				//Add the MIS heuristic
				real weight = powerHeuristic(lightInfos.pdf, bsdfPDF);

				radiance += throughput * value * bsdf->eval(bsdfInfos) * cosTheta * weight;
				if (radiance.isNan())
					std::cout << "nan 1 " << std::endl;

				if (DifferentialGeometry::cosTheta(localWi) <= 0.)
					shadowCaught = false; //true
				else
					shadowCaught = false;
			}
			else
			{
				shadowCaught = true;
			}
		}
		//std::cout << shadowCaught << std::endl;
		//BSDF sampling strategy
		Vector3d localWi = intersection.toLocal(-ray.direction());
		BSDFSamplingInfos bsdfInfos(intersection, localWi);
		bsdfInfos.uv = intersection.myUv; //
		bsdfInfos.shadowCaught = shadowCaught;
		Point2d s = sampler->getNextSample2D();
		Color bsdfValue = bsdf->sample(bsdfInfos, s);
		
		if (bsdfValue.isZero())
			break;

		Intersection toLightInter;
		//Go to world coord to "orientate" the hemisphere
		Ray reflected(intersection.myPoint, intersection.toWorld(bsdfInfos.wo));//, ray.myMinT, ray.myMaxT); //min, max, ok ?
		//std::cout << depth << " " << intersection.myPoint << " " << ray.direction() << " " << reflected.direction() << std::endl;
		Light::ptr lightCaught;
		Color radianceLight;
		//Trace a ray from the inter point to the bsdf sampled direction
		if (scene.computeIntersection(reflected, toLightInter))
		{
			//If we caught a light
			if (toLightInter.myPrimitive->isLight())
			{
				lightCaught = toLightInter.myPrimitive->getLight();
				radianceLight = lightCaught->le(-reflected.direction(), toLightInter.myPoint, toLightInter.myShadingGeometry.myN);
			}
		}
		else
		{
			lightCaught = scene.getEnvironmentLight();
			if (lightCaught == nullptr)
				return radiance;

			radianceLight = lightCaught->le(reflected.direction());
		}

		if (lightCaught != nullptr)
		{
			LightSamplingInfos lightInfos;
			lightInfos.normal = toLightInter.myShadingGeometry.myN; //to change
			lightInfos.sampledPoint = toLightInter.myPoint; //to change
			lightInfos.interToLight = reflected.direction();
			//Light::ptr light = toLightInter.myPrimitive->getLight();

			//pbrt book p 750 :  light'pdf is 0 if BSDF is specular because there is no chance of 
			//the light sampling the specular direction
			//Or, better : ompf2.com : MIS Question. BRDF is 0 for specular materials. So direct light 
			//sampling with MIS is not valid for specular (delta) mats. The only valid technique is
			//to sample the BRDF. Since there is only one valid technique, MIS cannot be applied 
			//(ie light pdf = 0)
			real pdfLight = (bsdfInfos.sampledType & BSDF::DELTA) ? 0.f : lightCaught->pdf(intersection.myPoint, lightInfos);
			real weight = powerHeuristic(bsdfInfos.pdf, pdfLight);

			//Color radianceLight = lightCaught->le(-reflected.direction(), toLightInter-> myPoint, toLightInter.myShadingGeometry.myN);
			radiance += throughput * radianceLight * bsdfValue * weight;
			//std::cout << depth << " " << radiance << " " << throughput << " " << radianceLight << " " << bsdfValue << " " << weight << std::endl;
			if (radiance.isNan()) {
				std::cout << "nan 2 " << throughput << " " << radianceLight << " " << bsdfValue << " " << weight << std::endl;
				lightCaught->pdf(intersection.myPoint, lightInfos);
				scene.computeIntersection(reflected, toLightInter);
			}

			if (lightCaught == scene.getEnvironmentLight())
				//noIntersection = true;
				return radiance;
		}

		throughput *= bsdfValue;
		eta *= bsdfInfos.relativeEta;

		if (depth >= myMinDepth)
		{
			
			real stopVal = std::min((real)0.95, throughput.luminance() * eta * eta);
			if (sampler->getNextSample1D() < stopVal)
			{
				throughput /= stopVal;
			}
			else
			{
				//Light::ptr tmp = scene.getEnvironmentLight();
				//if (tmp == nullptr)
				//	return radiance;

				//radianceLight = tmp->le(reflected.direction());
				//radiance += throughput * radianceLight;
				//const Scene::LightVector& lights = scene.getLights();
				//Light::ptr envLight = scene.getEnvironmentLight();
			
				//for (Light::ptr light : lights)
				//{
				//	if (light == envLight)
				//		radiance += throughput * light->le(ray.direction());
				//	else
				//		radiance += throughput * light->le(-ray.direction());
				//}
				return radiance;
			}
		}
		intersection = toLightInter;
		ray = reflected;

		radianceType = RadianceType::NO_EMISSION;

		depth++;
	}

	return radiance;
}

RT_REGISTER_TYPE(PathTracingMIS, Integrator)










/*

#include "PathTracingMIS.h"

#include "Parameters.h"
#include "Intersection.h"
#include "Scene.h"

#include "Mapping.h"

PathTracingMIS::PathTracingMIS(const Parameters& params)
{
	std::string sStrategy = params.getString("strategy", lightStrategy::STRING[LightSamplingStrategy::ONE_LIGHT_UNIFORM]);
	myStrategy = myStrategiesByName[sStrategy];

	myMaxDepth = params.getInt("maxDepth", 1000);
	myAOLength = params.getInt("aoLength", 100);
}


PathTracingMIS::~PathTracingMIS()
{
}

Color PathTracingMIS::li(Scene & scene, Sampler::ptr sampler, const Ray & _ray)
{
	//std::cout << "START" << std::endl;
	Color radiance;
	const real proba = 0.8;
	Color throughput(1.);
	real eta = 1.;
	int depth = 0;
	Ray ray(_ray);
	bool shadowCaught = false;
	real ao = 1.;

	Intersection intersection;
	if (!scene.computeIntersection(ray, intersection))
	{
		Light::ptr envLight = scene.getEnvironmentLight();
		if (envLight != nullptr)
		{
			radiance += envLight->le(ray.direction());
		}

		return radiance;
	}
	bool noIntersection = false;

	while (depth < myMaxDepth)
	{
		//if (noIntersection)
		//{
		//	Light::ptr envLight = scene.getEnvironmentLight();
		//	if (envLight != nullptr)
		//	{
		//		radiance += throughput * envLight->le(ray.direction());
		//	}

		//	return radiance;
		//}

		BSDF::ptr bsdf = intersection.myPrimitive->getBSDF();

		if (bsdf == nullptr)
			return radiance;

		//If we have intersected a light, add the radiance
		if (depth == 0 && intersection.myPrimitive->isLight())
			radiance += throughput * intersection.myPrimitive->le(-ray.direction(), intersection.myShadingGeometry.myN);

		//Light sampling strategy
		if (myStrategy == LightSamplingStrategy::ALL_LIGHT)
		{
			LightSamplingInfos lightInfos;
			const Scene::LightVector& lights = scene.getLights();
			for (unsigned int i = 0; i < lights.size(); i++)
			{
				Color value = sampleOneLight(lights[i], intersection.myPoint, sampler->getNextSample2D(), scene, lightInfos);
				if (!value.isZero())
				{

					Vector3d localWi = intersection.toLocal(lightInfos.interToLight);
					Vector3d localWo = intersection.toLocal(-ray.direction());
					real cosTheta = DifferentialGeometry::cosTheta(localWi);

					BSDFSamplingInfos bsdfInfos(intersection, localWi, localWo);
					bsdfInfos.uv = intersection.myUv; //

					//Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))

					//Same as specular BSDF : delta lights cannot be intersected during
					//BSDF sampling. Then, MIS is not applyable and the "weight" is 1
					real bsdfPDF = lights[i]->isDelta() ? 0. : bsdf->pdf(bsdfInfos);
					//Add the MIS heuristic
					real weight = powerHeuristic(lightInfos.pdf, bsdfPDF);

					radiance += throughput * value * bsdf->eval(bsdfInfos) * cosTheta * weight;

					if (DifferentialGeometry::cosTheta(localWi) <= 0.)
						shadowCaught = true;
					else
						shadowCaught = false;
				}
				else
				{
					shadowCaught = true;
				}
			}
		}
		else
		{
			if (bsdf->isShadowCatcher())
			{
				int cpt = 0;
				for (int k = 0; k < 64; k++)
				{
					Vector3d newDirection = Mapping::uniformSampleHemisphere(sampler, intersection.myShadingGeometry.myN);
					Ray newRay(intersection.myPoint, newDirection, tools::EPSILON, myAOLength);

					Intersection inter;
					if (!scene.computeIntersection(newRay, inter, true))
						cpt++;
				}
				ao = cpt / 64.;
			}
			else
			{
				LightSamplingInfos lightInfos;
				Color value = sampleLightDirect(intersection.myPoint, sampler->getNextSample2D(), scene, lightInfos, myStrategy);
				if (!value.isZero())
				{
					Vector3d localWi = intersection.toLocal(lightInfos.interToLight);
					Vector3d localWo = intersection.toLocal(-ray.direction());
					real cosTheta = DifferentialGeometry::cosTheta(localWi);

					BSDFSamplingInfos bsdfInfos(intersection, localWi, localWo);
					bsdfInfos.uv = intersection.myUv;

					//Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))

					//Same as specular BSDF : delta lights cannot be intersected during
					//BSDF sampling. Then, MIS is not applyable and the "weight" is 1
					real bsdfPDF = lightInfos.light->isDelta() ? 0. : bsdf->pdf(bsdfInfos);
					//Add the MIS heuristic
					real weight = powerHeuristic(lightInfos.pdf, bsdfPDF);

					radiance += throughput * value * bsdf->eval(bsdfInfos) * cosTheta * weight;
					if (radiance.isNan())
						std::cout << "nan 1 " << std::endl;

					if (DifferentialGeometry::cosTheta(localWi) <= 0.)
						shadowCaught = true;
					else
						shadowCaught = false;
				}
				else
				{
					shadowCaught = true;
				}
			}
		}
		//std::cout << shadowCaught << std::endl;
		//BSDF sampling strategy
		Vector3d localWi = intersection.toLocal(-ray.direction());
		BSDFSamplingInfos bsdfInfos(intersection, localWi);
		bsdfInfos.uv = intersection.myUv; //
		bsdfInfos.shadowCaught = shadowCaught;
		bsdfInfos.ao = ao;
		Color bsdfValue = bsdf->sample(bsdfInfos, sampler->getNextSample2D());

		if (bsdfValue.isZero())
			break;

		Intersection toLightInter;
		//Go to world coord to "orientate" the hemisphere
		Ray reflected(intersection.myPoint, intersection.toWorld(bsdfInfos.wo));//, ray.myMinT, ray.myMaxT); //min, max, ok ?
		//std::cout << depth << " " << intersection.myPoint << " " << ray.direction() << " " << reflected.direction() << std::endl;
		Light::ptr lightCaught;
		Color radianceLight;
		//Trace a ray from the inter point to the bsdf sampled direction
		if (scene.computeIntersection(reflected, toLightInter))
		{
			//If we caught a light
			if (toLightInter.myPrimitive->isLight())
			{
				lightCaught = toLightInter.myPrimitive->getLight();
				radianceLight = lightCaught->le(-reflected.direction(), toLightInter->myPoint, toLightInter.myShadingGeometry.myN);
			}
		}
		else
		{
			lightCaught = scene.getEnvironmentLight();
			if (lightCaught == nullptr)
				return radiance;

			radianceLight = lightCaught->le(reflected.direction()) * bsdfInfos.ao;
		}

		if (lightCaught != nullptr)
		{
			LightSamplingInfos lightInfos;
			lightInfos.normal = toLightInter.myShadingGeometry.myN; //to change
			lightInfos.sampledPoint = toLightInter.myPoint; //to change
			lightInfos.interToLight = reflected.direction();
			//Light::ptr light = toLightInter.myPrimitive->getLight();

			//pbrt book p 750 :  light'pdf is 0 if BSDF is specular because there is no chance of
			//the light sampling the specular direction
			//Or, better : ompf2.com : MIS Question. BRDF is 0 for specular materials. So direct light
			//sampling with MIS is not valid for specular (delta) mats. The only valid technique is
			//to sample the BRDF. Since there is only one valid technique, MIS cannot be applied
			//(ie light pdf = 0)
			real pdfLight = (bsdfInfos.sampledType & BSDF::DELTA) ? 0. : lightCaught->pdf(intersection.myPoint, lightInfos);
			real weight = powerHeuristic(bsdfInfos.pdf, pdfLight);

			//Color radianceLight = lightCaught->le(-reflected.direction(), toLightInter.myPoint, toLightInter.myShadingGeometry.myN);
			radiance += throughput * radianceLight * bsdfValue * weight;
			//std::cout << depth << " " << radiance << " " << throughput << " " << radianceLight << " " << bsdfValue << " " << weight << std::endl;
			if (radiance.isNan())
				std::cout << "nan 2 " << throughput << " " << radianceLight << " " << bsdfValue << " " << weight << std::endl;

			if (lightCaught == scene.getEnvironmentLight())
				//noIntersection = true;
				return radiance;
		}

		throughput *= bsdfValue;
		eta *= bsdfInfos.relativeEta;

		if (depth >= 0)
		{

			real stopVal = 0.9;//std::min(0.9, throughput.luminance() * eta * eta);
			if (sampler->getNextSample1D() < stopVal)
			{
				throughput /= stopVal;
			}
			else
			{
				//Light::ptr tmp = scene.getEnvironmentLight();
				//if (tmp == nullptr)
				//	return radiance;

				//radianceLight = tmp->le(reflected.direction());
				//radiance += throughput * radianceLight;
				//const Scene::LightVector& lights = scene.getLights();
				//Light::ptr envLight = scene.getEnvironmentLight();

				//for (Light::ptr light : lights)
				//{
				//	if (light == envLight)
				//		radiance += throughput * light->le(ray.direction());
				//	else
				//		radiance += throughput * light->le(-ray.direction());
				//}
				return radiance;
			}
		}
		intersection = toLightInter;
		ray = reflected;

		depth++;
	}

	return radiance;
}

RT_REGISTER_TYPE(PathTracingMIS, Integrator)

*/