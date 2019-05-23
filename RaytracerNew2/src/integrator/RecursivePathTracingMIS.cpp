#include "RecursivePathTracingMIS.h"

#include "bsdf/BSDF.h"
#include "core/Intersection.h"
#include "factory/ObjectFactoryManager.h"
#include "core/Scene.h"

RecursivePathTracingMIS::RecursivePathTracingMIS(const Parameters& params)
{
	myStrategy = LightSamplingStrategy::ONE_LIGHT_UNIFORM;// params.getString("strategy", "uniform");
	myMaxDepth = params.getInt("maxDepth", 1000);
}


RecursivePathTracingMIS::~RecursivePathTracingMIS()
{
}

Color RecursivePathTracingMIS::li(Scene & scene, Sampler::ptr sampler, const Ray & ray, RadianceType::ERadianceType radianceType)
{
	Intersection intersection;
	Color throughput(1.);
	real eta = 1.;
	return li(scene, sampler, ray, 0, intersection, throughput, eta);
}

//Radiance type is not used and should be used !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Color RecursivePathTracingMIS::li(Scene & scene, Sampler::ptr sampler, const Ray & ray, int depth, const Intersection& inter, Color& throughput, real& eta)
{
	Color radiance;
	const real proba = 0.8f;

	Intersection intersection = inter;
	if (depth == 0 && !scene.computeIntersection(ray, intersection))
	{
		Light::ptr envLight = scene.getEnvironmentLight();
		if (envLight != nullptr)
		{
			radiance += envLight->le(ray.direction(), Point3d(), Normal3d());
		}

		return radiance;
	}

	BSDF::ptr bsdf = intersection.myPrimitive->getBSDF();

	if (bsdf == nullptr)
		return radiance;

	//If we have intersected a light, add the radiance
	if (depth == 0 && intersection.myPrimitive->isLight())
		radiance += intersection.myPrimitive->le(-ray.direction(), intersection.myPoint, intersection.myShadingGeometry.myN);

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

				BSDFSamplingInfos bsdfInfos(localWi, localWo);
				bsdfInfos.uv = intersection.myUv; //

				//Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))
				//Add the MIS heuristic
				//Same as specular BSDF : delta lights cannot be intersected during 
				//BSDF sampling. Then, MIS is not applyable and the "weight" is 1
				real bsdfPDF = lights[i]->isDelta() ? 0.f : bsdf->pdf(bsdfInfos);
				real weight = powerHeuristic(lightInfos.pdf, bsdfPDF);

				radiance += value * bsdf->eval(bsdfInfos) * cosTheta * weight;
			}
		}
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
			real bsdfPDF = lightInfos.light->isDelta() ? 0.f : bsdf->pdf(bsdfInfos);
			//Add the MIS heuristic
			real weight = powerHeuristic(lightInfos.pdf, bsdfPDF);//bsdf->pdf(bsdfInfos));

			radiance += value * bsdf->eval(bsdfInfos) * cosTheta * weight;
			if (radiance.isNan())
				std::cout << "nan 1 " << std::endl;
		}
	}
	

	//Light sampling strategy
	//For all the lights
	//const Scene::LightVector& lights = scene.getLights();
	////for (unsigned int i = 0; i < lights.size(); i++)
	//{
	//	Light::ptr light = lights[(int)(sampler->getNextSample1D() * lights.size())];
	//	//Sample the light
	//	LightSamplingInfos lightInfos = light->sample(intersection.myPoint, sampler->getNextSample2D());

	//	//If there is no luminance associated, pass
	//	//if (lightInfos.intensity.luminance() <= 1e-7)
	//	//	continue;

	//	//Generate a shadow ray from the first inter to the light sampled point
	//	Vector3d interToLight = lightInfos.sampledPoint - intersection.myPoint;
	//	Ray shadowRay(intersection.myPoint, lightInfos.interToLight, tools::EPSILON, interToLight.norm() - tools::EPSILON);
	//	Intersection tmp;

	//	//If the light point is visible from the the inter point
	//	if (lightInfos.intensity.luminance() > 0 && !scene.computeIntersection(shadowRay, tmp, true))
	//	{
	//		Vector3d localWi = intersection.toLocal(lightInfos.interToLight);
	//		Vector3d localWo = intersection.toLocal(-ray.direction());
	//		real cosTheta = DifferentialGeometry::cosTheta(localWi);

	//		BSDFSamplingInfos bsdfInfos(localWi, localWo);

	//		//Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))
	//		//Add the MIS heuristic
	//		real weight = powerHeuristic(lightInfos.pdf, bsdf->pdf(bsdfInfos));

	//		if (bsdf && lightInfos.pdf > 0) {
	//			radiance += lights.size() * (lightInfos.intensity * bsdf->eval(bsdfInfos) * cosTheta * weight) / (lightInfos.pdf);
	//			if (std::isnan(radiance.r))
	//				std::cout << "nan 1 " << std::endl;
	//		}
	//	}
	//}

	//BSDF sampling strategy
	Vector3d localWi = intersection.toLocal(-ray.direction());
	BSDFSamplingInfos bsdfInfos(intersection, localWi);
	bsdfInfos.uv = intersection.myUv; //
	Color bsdfValue = bsdf->sample(bsdfInfos, sampler->getNextSample2D()); 
	if (bsdfValue.isZero())
		return radiance;

	Intersection toLightInter;
	//Go to world coord to "orientate" the hemisphere
	Ray reflected(intersection.myPoint, intersection.toWorld(bsdfInfos.wo));//, ray.myMinT, ray.myMaxT); //min, max, ok ?

	//Trace a ray from the inter point to the bsdf sampled direction
	if (scene.computeIntersection(reflected, toLightInter))
	{
		//If we caught a light
		if (toLightInter.myPrimitive->isLight())
		{
			LightSamplingInfos lightInfos;
			lightInfos.normal = toLightInter.myShadingGeometry.myN;
			lightInfos.sampledPoint = toLightInter.myPoint; //and wi ??????????
			Light::ptr light = toLightInter.myPrimitive->getLight();

			//pbrt book p 750 :  light'pdf is 0 is BSDF is specular because there is no chance of 
			//the light sampling the specular direction
			//Or, better : ompf2.com : MIS Question. BRDF is 0 for specular materials. So direct light 
			//sampling with MIS is not valid for specular (delta) mats. The only valid technique is
			//to sample the BRDF. Since there is only one valid technique, MIS cannot be applied 
			//(ie light pdf = 0)
			real pdfLight = (bsdfInfos.sampledType & BSDF::DELTA) ? 0.f : light->pdf(intersection.myPoint, lightInfos);
			real weight = powerHeuristic(bsdfInfos.pdf, pdfLight);

			Color radianceLight = toLightInter.myPrimitive->le(-reflected.direction(), toLightInter.myPoint, toLightInter.myShadingGeometry.myN);
			radiance += radianceLight * bsdfValue * weight;
			if(radiance.isNan())
				std::cout << "nan 2 " << std::endl;
		}
	}
	else
	{
		LightSamplingInfos lightInfos;
		lightInfos.interToLight = reflected.direction(); //check
		Light::ptr envLight = scene.getEnvironmentLight();
		if (envLight != nullptr)
		{
			Color lightValue = envLight->le(reflected.direction(), Point3d(), Normal3d());
			real pdfLight = (bsdfInfos.sampledType & BSDF::DELTA) ? 0.f : envLight->pdf(intersection.myPoint, lightInfos);
			real weight = powerHeuristic(bsdfInfos.pdf, pdfLight); //check pdf envlight = 0
			if (!std::isnan(weight))
				radiance += lightValue * bsdfValue * weight;
		}

		return radiance;
	}
	
	throughput *= bsdfValue;
	eta *= bsdfInfos.relativeEta;

	if (depth > 3 && depth < myMaxDepth)
	{
		depth++;
		real stopVal = 0.8f;//std::min(0.9, throughput.luminance() * eta * eta);
		if (sampler->getNextSample1D() < stopVal)//proba)
		{
			throughput /= stopVal;
			//if(bsdf->pdf(bsdfInfos) == 0.)
				radiance += bsdfValue * li(scene, sampler, reflected, depth, toLightInter, throughput, eta) / (stopVal);// *bsdf->pdf(bsdfInfos));
				
			//else
			//	radiance += bsdfValue * li(scene, sampler, _ray, depth) / (proba *bsdf->pdf(bsdfInfos));
		}
		else
		{
			return radiance;
		}
	}
	else if(depth < myMaxDepth)
	{
		depth++;
		radiance += bsdfValue * li(scene, sampler, reflected, depth, toLightInter, throughput, eta);
	}

	return radiance;
}

RT_REGISTER_TYPE(RecursivePathTracingMIS, Integrator)




















//Color PathTracingMIS::li(Scene & scene, Sampler::ptr sampler, const Ray & _ray)
//{
//	Color radiance;
//	const real proba = 0.8;
//	Color throughput(1.);
//	Intersection intersection;
//	Ray ray = _ray;
//	for (int depth = 0; depth < 1000; depth++)
//	{
//		if (/*depth == 0 &&*/ !scene.computeIntersection(ray, intersection))
//		{
//			Light::ptr envLight = scene.getEnvironmentLight();
//			if (envLight != nullptr)
//			{
//				radiance += envLight->le(ray.direction(), Point3d(), Normal3d());
//			}
//
//			return radiance;
//		}
//
//		BSDF::ptr bsdf = intersection.myPrimitive->getBSDF();
//
//		if (bsdf == nullptr)
//			return radiance;
//
//		//If we have intersected a light, add the radiance
//		if (depth == 0 && intersection.myPrimitive->isLight())
//			radiance += intersection.myPrimitive->le(-ray.direction(), intersection.myPoint, intersection.myShadingGeometry.myN);
//
//		//Light sampling strategy
//		//For all the lights
//		const Scene::LightVector& lights = scene.getLights();
//		for (unsigned int i = 0; i < lights.size(); i++)
//		{
//			//Light::ptr light = lights[(int)(sampler->getNextSample1D() * lights.size())];
//			//Sample the light
//			LightSamplingInfos lightInfos = lights[i]->sample(intersection.myPoint, sampler);
//
//			//If there is no luminance associated, pass
//			if (lightInfos.intensity.luminance() <= 1e-7)
//				continue;
//
//			//Generate a shadow ray from the first inter to the light sampled point
//			Vector3d interToLight = lightInfos.samplingPoint - intersection.myPoint;
//			Ray shadowRay(intersection.myPoint, lightInfos.wi, tools::EPSILON, interToLight.norm() - tools::EPSILON);
//			Intersection tmp;
//
//			//If the light point is visible from the the inter point
//			if (lightInfos.intensity.luminance() > 0 && !scene.computeIntersection(shadowRay, tmp, true))
//			{
//				Vector3d localWi = intersection.toLocal(lightInfos.wi);
//				Vector3d localWo = intersection.toLocal(-ray.direction());
//				real cosTheta = DifferentialGeometry::cosTheta(localWi);
//
//				BSDFSamplingInfos bsdfInfos(localWi, localWo);
//
//				//Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))
//				//Add the MIS heuristic
//				real weight = powerHeuristic(0.5, lightInfos.pdf, 0.5, bsdf->pdf(bsdfInfos));
//
//				if (bsdf && lightInfos.pdf > 0) {
//					radiance += throughput * (lightInfos.intensity * bsdf->eval(bsdfInfos) * cosTheta * weight) / (lightInfos.pdf);
//					if (std::isnan(radiance.r) || lightInfos.pdf <= 0)
//						std::cout << "nan 1 " << std::endl;
//				}
//			}
//		}
//
//		//BSDF sampling strategy
//		Vector3d localWi = intersection.toLocal(-ray.direction());
//		BSDFSamplingInfos bsdfInfos(localWi);
//		Color bsdfValue = bsdf->sample(bsdfInfos, sampler->getNextSample2D()); //To change sampler
//		if (bsdfValue.isZero())
//			return radiance;
//
//		Intersection toLightInter;
//		//Go to world coord to "orientate" the hemisphere
//		Ray reflected(intersection.myPoint, intersection.toWorld(bsdfInfos.wo));
//		ray = reflected;
//		throughput *= bsdfValue;
//
//		//Trace a ray from the inter point to the bsdf sampled direction
//		if (scene.computeIntersection(reflected, toLightInter))
//		{
//			//If we caught a light
//			if (toLightInter.myPrimitive->isLight())
//			{
//				LightSamplingInfos lightInfos;
//				lightInfos.normal = toLightInter.myShadingGeometry.myN;
//				lightInfos.samplingPoint = toLightInter.myPoint; //and wi ??????????
//				Light::ptr light = toLightInter.myPrimitive->getLight();
//				real weight = powerHeuristic(0.5, bsdfInfos.pdf, 0.5, light->pdf(intersection.myPoint, lightInfos));
//
//				Color radianceLight = toLightInter.myPrimitive->le(-reflected.direction(), toLightInter.myPoint, toLightInter.myShadingGeometry.myN);
//				radiance += throughput * radianceLight * bsdfValue * weight;
//				if (std::isnan(radiance.r))
//					std::cout << "nan 2 " << std::endl;
//			}
//		}
//		else
//		{
//			LightSamplingInfos lightInfos;
//			lightInfos.wi = reflected.direction(); //check
//			Light::ptr envLight = scene.getEnvironmentLight();
//			if (envLight != nullptr)
//			{
//				Color lightValue = envLight->le(reflected.direction(), Point3d(), Normal3d());
//				real weight = powerHeuristic(0.5, bsdfInfos.pdf, 0.5, envLight->pdf(intersection.myPoint, lightInfos)); //check pdf envlight = 0
//				if (!std::isnan(weight))
//					radiance += throughput * lightValue * bsdfValue * weight;
//			}
//			else
//				return radiance;
//		}
//
//
//
//		if (depth > 3)
//		{
//
//			real stopVal = std::min(0.8, throughput.luminance());
//			if (sampler->getNextSample1D() < stopVal)//proba)
//			{
//				throughput /= stopVal;
//
//			}
//			else
//			{
//				return radiance;
//			}
//		}
//	}
//
//	return radiance;
//}