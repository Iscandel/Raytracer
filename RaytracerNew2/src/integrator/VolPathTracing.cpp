#include "VolPathTracing.h"



#include "core/Parameters.h"
#include "core/Intersection.h"
#include "core/Scene.h"

VolPathTracing::VolPathTracing(const Parameters& params)
	//:myMedium(Parameters())
{
	std::string sStrategy = params.getString("strategy", lightStrategy::STRING[LightSamplingStrategy::ONE_LIGHT_UNIFORM]);
	myStrategy = myStrategiesByName[sStrategy];

	myMinDepth = params.getInt("minDepth", 3);
	myMaxDepth = params.getInt("maxDepth", 1000);
}


VolPathTracing::~VolPathTracing()
{
}

Color VolPathTracing::li(Scene & scene, Sampler::ptr sampler, const Ray & _ray, RadianceType::ERadianceType radianceType)
{
	Color radiance;
	const real proba = 0.8f;
	Color throughput(1.f);
	real eta = 1.f;
	int depth = 0;
	Ray ray(_ray);
	bool shadowCaught = false;
	Intersection intersection;
	bool directRadiance = true;
	bool wasIntersected = scene.computeIntersection(ray, intersection);
	//{
	//	Light::ptr envLight = scene.getEnvironmentLight();
	//	if (envLight != nullptr)
	//	{
	//		radiance += envLight->le(ray.direction());
	//	}

	//	return radiance;
	//}
	

	//Go to world coord to "orientate" the hemisphere
	bool fromMedia = false;

	Medium::ptr medium = scene.getCameraMedium();

	while (depth < myMaxDepth)
	{
		//thread_local int t;
		real t;
		Color weightMedium;
		Color emissivityMedium;
		//ray.myMaxT = intersection.t;

		if (medium && medium->sampleDistance(Ray(ray.myOrigin, ray.direction(), ray.myMinT, intersection.t), sampler, t, weightMedium, emissivityMedium))
		{
			radiance += throughput * emissivityMedium;

			throughput *= weightMedium;

			//radiance += weightMedium * throughput + emissivityMedium;
			//throughput *= weightMedium + emissivityMedium;
			//std::cout << weightMedium << std::endl;
			fromMedia = true;

			//Change origin of the ray
			//intersection.t = t;
			intersection.myPoint = ray.getPointAt(t);

			/////////////////////////////////////////////
			//Light sampling
			LightSamplingInfos lightInfos;
			Color value = sampleAttenuatedLightDirect(intersection.myPoint, sampler, scene, lightInfos, medium);

			if (!value.isZero())
			{
				PhaseFunctionSamplingInfos phaseInfos(lightInfos.interToLight, -ray.direction());

				real pfPDF = medium->pdfPF(phaseInfos); 
				pfPDF = lightInfos.light->isDelta() ? 0.f : pfPDF;
				real weight = powerHeuristic(lightInfos.pdf, pfPDF);

				radiance += throughput * value * medium->evalPF(phaseInfos) * weight;

				if (radiance.isNan())
					std::cout << "nan 0 " << throughput << " " << value << std::endl;
			}

			/////////////////////////////////////////////
			//Phase sampling
			PhaseFunctionSamplingInfos pfInfos(-ray.direction());
			real pfWeight = medium->samplePF(pfInfos, sampler->getNextSample2D());

			ray.myOrigin = intersection.myPoint;
			ray.direction(pfInfos.wo);
			ray.myMinT = 0;

			Medium::ptr newMedium;
			Color tr = evalTransmittanceFromSampledBSDF(scene, ray, sampler, medium, intersection, wasIntersected, newMedium);

			throughput *= pfWeight;

			//wasIntersected = scene.computeIntersection(ray, intersection);
			if(!wasIntersected)
			{
				//Light::ptr envLight = scene.getEnvironmentLight();
				//if (envLight != nullptr)
				//{
				//	radiance += throughput * envLight->le(ray.direction()); //tr ?
				//}

				//return radiance;
			}
			else
			{
				//If we caught a light
				if (intersection.myPrimitive->isLight())
				{
					Light::ptr lightCaught = intersection.myPrimitive->getLight();
					Color radianceLight = lightCaught->le(-ray.direction(), intersection.myShadingGeometry.myN);

					//Color tr = medium->transmittance(Ray(ray.myOrigin, ray.direction(), 0., intersection.t), sampler);
																		
					real pdfLight = lightCaught->pdf(intersection.myPoint, lightInfos);
					real weight = powerHeuristic(pfInfos.pdf, pdfLight);
					//std::cout << pfPDF << " " << weight << " " << radianceLight << std::endl;
					radiance += throughput * radianceLight * tr * weight;
				}
			}
		}
		else
		{
			if (medium)
			{
				radiance += throughput * emissivityMedium;
				throughput *= weightMedium;
			}

			//Should not happen. Medium should be bounded, especially with env maps
			if (!wasIntersected)
			{
				Light::ptr envLight = scene.getEnvironmentLight();
				if (envLight != nullptr)
				{
					radiance += throughput * envLight->le(ray.direction()); //tr ?
				}

				return radiance;
			}

			fromMedia = false;
			BSDF::ptr bsdf = intersection.myPrimitive->getBSDF();

			if (bsdf == nullptr)
			{
				ray = Ray(intersection.myPoint, ray.direction());
				if (intersection.isMediumTransition())
					medium = intersection.getMedium(ray);
				wasIntersected = scene.computeIntersection(ray, intersection);
				directRadiance = true;
				continue;
			}

			//If we have intersected a light, add the radiance
			if ((depth == 0 /*|| directRadiance*/) && intersection.myPrimitive->isLight()) {// || (fromMedia && intersection.myPrimitive->isLight()))
				radiance += throughput * intersection.myPrimitive->le(-ray.direction(), intersection.myShadingGeometry.myN); //tr ?
				directRadiance = false;
			}
			
			//Light sampling strategy
			LightSamplingInfos lightInfos;
			Color value = sampleAttenuatedLightDirect(intersection.myPoint, sampler, scene, lightInfos, medium);
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
					std::cout << "nan 1 " << throughput << " " << value << " " << bsdf->eval(bsdfInfos) << " " << localWi << " " << cosTheta << " " << weight << std::endl;

				if (DifferentialGeometry::cosTheta(localWi) <= 0.)
					shadowCaught = true;
				else
					shadowCaught = false;
			}
			else
			{
				shadowCaught = true;
			}

			//std::cout << shadowCaught << std::endl;
			//BSDF sampling strategy
			Vector3d localWi = intersection.toLocal(-ray.direction());
			BSDFSamplingInfos bsdfInfos(intersection, localWi);
			bsdfInfos.uv = intersection.myUv; //
			bsdfInfos.shadowCaught = shadowCaught;
			Color bsdfValue = bsdf->sample(bsdfInfos, sampler->getNextSample2D());

			if (bsdfValue.isZero())
				break;

			//Intersection toLightInter;
			//Go to world coord to "orientate" the hemisphere
			ray = Ray(intersection.myPoint, intersection.toWorld(bsdfInfos.wo));

			//if we change the medium
			if (intersection.isMediumTransition())
				medium = intersection.getMedium(ray);
						
			Light::ptr lightCaught;
			Color radianceLight(0.f);
			Point3d pFrom = intersection.myPoint; //for the light pdf
			//Trace a ray from the inter point to the bsdf sampled direction

			Medium::ptr newMedium;
			Color tr = evalTransmittanceFromSampledBSDF(scene, ray, sampler, medium, intersection, wasIntersected, newMedium);
			
			if(wasIntersected)
			{
				//If we caught a light
				if (intersection.myPrimitive->isLight())
				{
					lightCaught = intersection.myPrimitive->getLight();
					radianceLight = lightCaught->le(-ray.direction(), intersection.myShadingGeometry.myN);
				}
			}
			else
			{
				//if (medium)
				//	ILogger::log() << "Should not happen. Medium must be bounded\n";
				if(!newMedium)
				{
					lightCaught = scene.getEnvironmentLight();
					//if (lightCaught == nullptr)
					//	return radiance;
					if (lightCaught != nullptr)
						radianceLight = lightCaught->le(ray.direction());
				}
			}

			if (lightCaught != nullptr)
			{
				LightSamplingInfos lightInfos;
				lightInfos.normal = intersection.myShadingGeometry.myN; //to change
				lightInfos.sampledPoint = intersection.myPoint; //to change
				lightInfos.interToLight = ray.direction();
				//Light::ptr light = toLightInter.myPrimitive->getLight();

				//pbrt book p 750 :  light'pdf is 0 if BSDF is specular because there is no chance of 
				//the light sampling the specular direction
				//Or, better : ompf2.com : MIS Question. BRDF is 0 for specular materials. So direct light 
				//sampling with MIS is not valid for specular (delta) mats. The only valid technique is
				//to sample the BRDF. Since there is only one valid technique, MIS cannot be applied 
				//(ie light pdf = 0)
				real pdfLight = (bsdfInfos.sampledType & BSDF::DELTA) ? 0.f : lightCaught->pdf(pFrom, lightInfos);
				real weight = powerHeuristic(bsdfInfos.pdf, pdfLight);

				//Color tr(1.);
				//if(medium)
				//	tr = medium->transmittance(Ray(ray.myOrigin, ray.direction(), 0., intersection.t), sampler);

				//std::cout << tr << std::endl;

				radiance += throughput * radianceLight * bsdfValue * tr * weight /** weightMedium*/;
				//std::cout << depth << " " << radiance << " " << throughput << " " << radianceLight << " " << bsdfValue << " " << weight << std::endl;
				if (radiance.isNan())
					std::cout << "nan 2 " << throughput << " " << radianceLight << " " << bsdfValue << " " << weight << std::endl;

				if (lightCaught == scene.getEnvironmentLight() && !medium)
					//noIntersection = true;
					return radiance;
			}

			throughput *= bsdfValue;
			eta *= bsdfInfos.relativeEta;

		}

		if (depth >= myMinDepth)
		{

			real stopVal = std::min((real)0.95, throughput.max() * eta * eta);
			if (sampler->getNextSample1D() < stopVal)
			{
				throughput /= stopVal;
			}
			else
			{
				return radiance;
			}
		}

		wasIntersected = scene.computeIntersection(ray, intersection);
		depth++;
	}


	return radiance;
}

Color VolPathTracing::evalTransmittanceFromSampledBSDF(Scene& scene, 
								const Ray& ray, Sampler::ptr sampler, Medium::ptr medium, Intersection& intersection, bool& wasIntersected, Medium::ptr& newMedium)
{
	Ray trRay(ray);
	Color tr(1.f);
	bool firstPass = true;
	wasIntersected = false;

	while (true)
	{
		wasIntersected = scene.computeIntersection(trRay, intersection);	
		
		//if (!wasIntersected && medium) {
		//	std::cout << "hum" << std::endl;
		//	wasIntersected = scene.computeIntersection(trRay, intersection);
		//}
				
		//Update transmittance
		if (medium)
			tr *= medium->transmittance(Ray(trRay.myOrigin, 
											trRay.direction(), 
											firstPass ? trRay.myMinT : 0.f, 
											wasIntersected ? intersection.t : trRay.myMaxT), sampler);

		if (wasIntersected && intersection.myPrimitive->getBSDF() != nullptr)
			return tr;

		firstPass = false;
		//No intersection, leave
		if (!wasIntersected || tr.isZero())
			return tr;

		//Update the ray
		trRay = Ray(trRay.getPointAt(intersection.t), trRay.direction(), math::EPSILON, trRay.myMaxT);

		if (intersection.isMediumTransition())
		{
			medium = intersection.getMedium(trRay);
			newMedium = medium;
		}
	}
}

RT_REGISTER_TYPE(VolPathTracing, Integrator)







//Works well, but NO MIS

//VolPathTracing::VolPathTracing(const Parameters& params)
////:myMedium(Parameters())
//{
//	std::string sStrategy = params.getString("strategy", lightStrategy::STRING[LightSamplingStrategy::ONE_LIGHT_UNIFORM]);
//	myStrategy = myStrategiesByName[sStrategy];
//
//	myMinDepth = params.getInt("minDepth", 3);
//	myMaxDepth = params.getInt("maxDepth", 1000);
//}
//
//
//VolPathTracing::~VolPathTracing()
//{
//}
//
//Color VolPathTracing::li(Scene & scene, Sampler::ptr sampler, const Ray & _ray)
//{
//	Color radiance;
//	const real proba = 0.8f;
//	Color throughput(1.f);
//	real eta = 1.f;
//	int depth = 0;
//	Ray ray(_ray);
//	bool shadowCaught = false;
//	Intersection intersection;
//	bool wasIntersected = scene.computeIntersection(ray, intersection);
//
//	//Go to world coord to "orientate" the hemisphere
//	bool fromMedia = false;
//
//	Medium::ptr medium = scene.getCameraMedium();
//
//	while (depth < myMaxDepth)
//	{
//		real t;
//		Color weightMedium;
//		//ray.myMaxT = intersection.t;
//
//		if (medium && medium->sampleDistance(Ray(ray.myOrigin, ray.direction(), ray.myMinT, intersection.t), sampler, t, weightMedium))
//		{
//			throughput *= weightMedium;
//			//std::cout << weightMedium << std::endl;
//			fromMedia = true;
//
//			//Change origin of the ray
//			//intersection.t = t;
//			intersection.myPoint = ray.getPointAt(t);
//
//			/////////////////////////////////////////////
//			//Light sampling
//			LightSamplingInfos lightInfos;
//			Color value = sampleAttenuatedLightDirect(intersection.myPoint, sampler, scene, lightInfos, medium);
//
//			if (!value.isZero())
//			{
//				PhaseFunctionSamplingInfos phaseInfos(lightInfos.interToLight, -ray.direction());
//
//				radiance += throughput * value * medium->evalPF(phaseInfos);
//
//				if (radiance.isNan())
//					std::cout << "nan 1 " << throughput << " " << value << std::endl;
//			}
//
//			/////////////////////////////////////////////
//			//Phase sampling
//			PhaseFunctionSamplingInfos pfInfos(-ray.direction());
//			real pfWeight = medium->samplePF(pfInfos, sampler->getNextSample2D());
//
//			ray.myOrigin = intersection.myPoint;
//			ray.direction(pfInfos.wo);
//			ray.myMinT = 0;
//
//			throughput *= pfWeight;
//
//			wasIntersected = scene.computeIntersection(ray, intersection);
//		}
//		else
//		{
//			if (medium)
//				throughput *= weightMedium;
//
//			//Should not happen. Medium should be bounded, especially with env maps
//			if (!wasIntersected)
//			{
//				Light::ptr envLight = scene.getEnvironmentLight();
//				if (envLight != nullptr)
//				{
//					radiance += throughput * envLight->le(ray.direction()); //tr ?
//				}
//
//				return radiance;
//			}
//
//			fromMedia = false;
//			BSDF::ptr bsdf = intersection.myPrimitive->getBSDF();
//
//			if (bsdf == nullptr)
//			{
//				ray = Ray(intersection.myPoint, ray.direction());
//				if (intersection.isMediumTransition())
//					medium = intersection.getMedium(ray);
//				wasIntersected = scene.computeIntersection(ray, intersection);
//				continue;
//			}
//
//			//If we have intersected a light, add the radiance
//			if (/*depth == 0 &&*/ intersection.myPrimitive->isLight())// || (fromMedia && intersection.myPrimitive->isLight()))
//				radiance += throughput * intersection.myPrimitive->le(-ray.direction(), intersection.myShadingGeometry.myN); //tr ?
//
//			//Light sampling strategy
//			LightSamplingInfos lightInfos;
//			Color value = sampleAttenuatedLightDirect(intersection.myPoint, sampler, scene, lightInfos, medium);
//			if (!value.isZero())
//			{
//				Vector3d localWi = intersection.toLocal(lightInfos.interToLight);
//				Vector3d localWo = intersection.toLocal(-ray.direction());
//				real cosTheta = DifferentialGeometry::cosTheta(localWi);
//
//				BSDFSamplingInfos bsdfInfos(intersection, localWi, localWo);
//				bsdfInfos.uv = intersection.myUv;
//
//				radiance += throughput * value * bsdf->eval(bsdfInfos) * cosTheta;
//				if (radiance.isNan())
//					std::cout << "nan 1 " << std::endl;
//			}
//
//			//BSDF sampling strategy
//			Vector3d localWi = intersection.toLocal(-ray.direction());
//			BSDFSamplingInfos bsdfInfos(intersection, localWi);
//			bsdfInfos.uv = intersection.myUv; //
//			bsdfInfos.shadowCaught = shadowCaught;
//			Color bsdfValue = bsdf->sample(bsdfInfos, sampler->getNextSample2D());
//
//			if (bsdfValue.isZero())
//				break;
//
//			//Intersection toLightInter;
//			//Go to world coord to "orientate" the hemisphere
//			ray = Ray(intersection.myPoint, intersection.toWorld(bsdfInfos.wo));
//
//			//if we change the medium
//			if (intersection.isMediumTransition())
//				medium = intersection.getMedium(ray);
//
//			throughput *= bsdfValue;
//			eta *= bsdfInfos.relativeEta;
//			
//			wasIntersected = scene.computeIntersection(ray, intersection);
//
//		}
//
//		if (depth >= myMinDepth)
//		{
//
//			real stopVal = std::min((real)0.95, throughput.max() * eta * eta);
//			if (sampler->getNextSample1D() < stopVal)
//			{
//				throughput /= stopVal;
//			}
//			else
//			{
//				return radiance;
//			}
//		}
//
//		
//
//		depth++;
//	}
//
//
//	return radiance;
//}


















//Ok, but crappy written


//VolPathTracing::VolPathTracing(const Parameters& params)
//	:myMedium(Parameters())
//{
//	std::string sStrategy = params.getString("strategy", lightStrategy::STRING[LightSamplingStrategy::ONE_LIGHT_UNIFORM]);
//	myStrategy = myStrategiesByName[sStrategy];
//
//	myMaxDepth = params.getInt("maxDepth", 1000);
//}
//
//
//VolPathTracing::~VolPathTracing()
//{
//}
//
//Color VolPathTracing::li(Scene & scene, Sampler::ptr sampler, const Ray & _ray)
//{
//	Color radiance;
//	const real proba = 0.8;
//	Color throughput(1.);
//	real eta = 1.;
//	int depth = 0;
//	Ray ray(_ray);
//	bool shadowCaught = false;
//	Intersection intersection;
//	if (!scene.computeIntersection(ray, intersection))
//	{
//		Light::ptr envLight = scene.getEnvironmentLight();
//		if (envLight != nullptr)
//		{
//			radiance += envLight->le(ray.direction());
//		}
//
//		return radiance;
//	}
//	bool noIntersection = false;
//	//ray.myMaxT = intersection.t;
//
//	Intersection toLightInter;
//	//Go to world coord to "orientate" the hemisphere
//	Ray reflected;
//	bool fromMedia = false;
//
//	while (depth < myMaxDepth)
//	{
//		//if (noIntersection)
//		//{
//		//	Light::ptr envLight = scene.getEnvironmentLight();
//		//	if (envLight != nullptr)
//		//	{
//		//		radiance += throughput * envLight->le(ray.direction());
//		//	}
//
//		//	return radiance;
//		//}
//
//		real t;
//		Color weightMedium;
//		Ray mediumRay(ray);
//		//ray.myMaxT = intersection.t;
//
//		//If we have intersected a light, add the radiance
//		if ((depth == 0 && intersection.myPrimitive->isLight()) || (fromMedia && intersection.myPrimitive->isLight()))
//			radiance += throughput * intersection.myPrimitive->le(-ray.direction(), intersection.myShadingGeometry.myN);
//
//		if (myMedium.sampleDistance(Ray(ray.myOrigin, ray.direction(), ray.myMinT, intersection.t), sampler->getNextSample2D(), t, weightMedium))
//		{
//			//fromMedia = true;
//			throughput *= weightMedium;
//
//			//Change origin of the ray
//			Ray tmpRay(ray);
//			intersection.myPoint = ray.getPointAt(t);
//			tmpRay.myOrigin = intersection.myPoint;
//
//			/////////////////////////////////////////////
//			//Light sampling
//			real lightWeight;
//			Light::ptr light;
//			const Scene::LightVector& lights = scene.getLights();
//			Point2d sample = sampler->getNextSample2D();
//
//			int index = myLightWeights.sampleAndReuse(sample.x(), lightWeight);
//			light = lights[index];
//			
//			//Sample the light
//			LightSamplingInfos lightInfos = light->sample(intersection.myPoint, sample);
//			lightInfos.light = light;
//
//			Ray shadowRay(intersection.myPoint, lightInfos.interToLight, tools::EPSILON, lightInfos.distance - tools::EPSILON);
//			Intersection tmp;
//
//			Color value(0.);
//			//If the light point is visible from the the inter point
//			if (lightInfos.pdf > 0 && lightInfos.intensity.luminance() > 0 && !scene.computeIntersection(shadowRay, tmp, true))
//			{
//				Color tr = myMedium.transmittance(shadowRay);
//				value = tr * lightInfos.intensity / lightInfos.pdf;
//			}
//
//			lightInfos.pdf *= lightWeight;
//
//			value /= lightWeight;
//			if (!value.isZero())
//			{
//				real pfPDF = myMedium.pdfPF(PhaseFunctionSamplingInfos(lightInfos.interToLight, -tmpRay.direction())); //to change
//				//Add the MIS heuristic
//				real weight = powerHeuristic(lightInfos.pdf, pfPDF);
//
//				radiance += throughput * value * myMedium.evalPF(PhaseFunctionSamplingInfos(lightInfos.interToLight, -tmpRay.direction())) * weight;
//
//				if (radiance.isNan())
//					std::cout << "nan 1 " << std::endl;
//			}
//
//			//Phase sampling
//			PhaseFunctionSamplingInfos pfInfos(-tmpRay.direction());
//			//pfInfos.wi = ray.direction()
//			real pfWeight = myMedium.samplePF(pfInfos, sampler->getNextSample2D());
//
//			tmpRay.direction(pfInfos.wo);
//			ray = tmpRay;
//
//			throughput *= pfWeight;
//
//			if (!scene.computeIntersection(ray, intersection))
//			{
//				Light::ptr envLight = scene.getEnvironmentLight();
//				if (envLight != nullptr)
//				{
//					radiance += throughput * envLight->le(ray.direction()); //tr ?
//				}
//
//				return radiance;
//			}
//			else
//			{
//				//If we caught a light
//				if (intersection.myPrimitive->isLight())
//				{
//					Light::ptr lightCaught = intersection.myPrimitive->getLight();
//					Color radianceLight = lightCaught->le(-ray.direction(), intersection.myShadingGeometry.myN);
//
//					Color tr = myMedium.transmittance(Ray(ray.myOrigin, ray.direction(), 0., intersection.t));
//					real pfPDF = pfInfos.pdf; //to change
//																				 //Add the MIS heuristic
//					real pdfLight = lightCaught->pdf(intersection.myPoint, lightInfos);
//					real weight = powerHeuristic(pfPDF, pdfLight);
//					//std::cout << pfPDF << " " << weight << " " << radianceLight << std::endl;
//					radiance += throughput * radianceLight * tr * weight;
//				}
//			}
//
//			//dirty
//			toLightInter = intersection;
//			reflected = ray;
//		}
//		else
//		{
//			fromMedia = false;
//			BSDF::ptr bsdf = intersection.myPrimitive->getBSDF();
//
//			if (bsdf == nullptr)
//				return radiance;
//
//
//
//			//Light sampling strategy
//			if (myStrategy == LightSamplingStrategy::ALL_LIGHT)
//			{
//				LightSamplingInfos lightInfos;
//				const Scene::LightVector& lights = scene.getLights();
//				for (unsigned int i = 0; i < lights.size(); i++)
//				{
//					Color value = sampleOneLight(lights[i], intersection.myPoint, sampler->getNextSample2D(), scene, lightInfos);
//					if (!value.isZero())
//					{
//
//						Vector3d localWi = intersection.toLocal(lightInfos.interToLight);
//						Vector3d localWo = intersection.toLocal(-ray.direction());
//						real cosTheta = DifferentialGeometry::cosTheta(localWi);
//
//						BSDFSamplingInfos bsdfInfos(intersection, localWi, localWo);
//						bsdfInfos.uv = intersection.myUv; //
//
//														  //Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))
//
//														  //Same as specular BSDF : delta lights cannot be intersected during 
//														  //BSDF sampling. Then, MIS is not applyable and the "weight" is 1
//						real bsdfPDF = lights[i]->isDelta() ? 0. : bsdf->pdf(bsdfInfos);
//						//Add the MIS heuristic
//						real weight = powerHeuristic(lightInfos.pdf, bsdfPDF);
//
//						radiance += throughput * value * bsdf->eval(bsdfInfos) * cosTheta * weight;
//
//						if (DifferentialGeometry::cosTheta(localWi) <= 0.)
//							shadowCaught = true;
//						else
//							shadowCaught = false;
//					}
//					else
//					{
//						shadowCaught = true;
//					}
//				}
//			}
//			else
//			{
//				throughput *= weightMedium;
//
//				LightSamplingInfos lightInfos;
//				Color value = sampleLightDirect(intersection.myPoint, sampler->getNextSample2D(), scene, lightInfos, myStrategy); //add transmittance
//				if (!value.isZero())
//				{
//					Vector3d localWi = intersection.toLocal(lightInfos.interToLight);
//					Vector3d localWo = intersection.toLocal(-ray.direction());
//					real cosTheta = DifferentialGeometry::cosTheta(localWi);
//
//					BSDFSamplingInfos bsdfInfos(intersection, localWi, localWo);
//					bsdfInfos.uv = intersection.myUv;
//
//					//Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))
//
//					//Same as specular BSDF : delta lights cannot be intersected during 
//					//BSDF sampling. Then, MIS is not applyable and the "weight" is 1
//					real bsdfPDF = lightInfos.light->isDelta() ? 0. : bsdf->pdf(bsdfInfos);
//					//Add the MIS heuristic
//					real weight = powerHeuristic(lightInfos.pdf, bsdfPDF);
//
//					radiance += throughput * value * bsdf->eval(bsdfInfos) * cosTheta * weight;
//					if (radiance.isNan())
//						std::cout << "nan 1 " << std::endl;
//
//					if (DifferentialGeometry::cosTheta(localWi) <= 0.)
//						shadowCaught = true;
//					else
//						shadowCaught = false;
//				}
//				else
//				{
//					shadowCaught = true;
//				}
//			}
//			//std::cout << shadowCaught << std::endl;
//			//BSDF sampling strategy
//			Vector3d localWi = intersection.toLocal(-ray.direction());
//			BSDFSamplingInfos bsdfInfos(intersection, localWi);
//			bsdfInfos.uv = intersection.myUv; //
//			bsdfInfos.shadowCaught = shadowCaught;
//			Color bsdfValue = bsdf->sample(bsdfInfos, sampler->getNextSample2D());
//
//			if (bsdfValue.isZero())
//				break;
//
//			//Intersection toLightInter;
//			//Go to world coord to "orientate" the hemisphere
//			reflected = Ray(intersection.myPoint, intersection.toWorld(bsdfInfos.wo));//, ray.myMinT, ray.myMaxT); //min, max, ok ?
//																					  //std::cout << depth << " " << intersection.myPoint << " " << ray.direction() << " " << reflected.direction() << std::endl;
//			Light::ptr lightCaught;
//			Color radianceLight;
//			//Trace a ray from the inter point to the bsdf sampled direction
//			if (scene.computeIntersection(reflected, toLightInter))
//			{
//				//If we caught a light
//				if (toLightInter.myPrimitive->isLight())
//				{
//					lightCaught = toLightInter.myPrimitive->getLight();
//					radianceLight = lightCaught->le(-reflected.direction(), toLightInter.myShadingGeometry.myN);
//				}
//			}
//			else
//			{
//				lightCaught = scene.getEnvironmentLight();
//				if (lightCaught == nullptr)
//					return radiance;
//
//				radianceLight = lightCaught->le(reflected.direction());
//			}
//
//			if (lightCaught != nullptr)
//			{
//				LightSamplingInfos lightInfos;
//				lightInfos.normal = toLightInter.myShadingGeometry.myN; //to change
//				lightInfos.sampledPoint = toLightInter.myPoint; //to change
//				lightInfos.interToLight = reflected.direction();
//				//Light::ptr light = toLightInter.myPrimitive->getLight();
//
//				//pbrt book p 750 :  light'pdf is 0 if BSDF is specular because there is no chance of 
//				//the light sampling the specular direction
//				//Or, better : ompf2.com : MIS Question. BRDF is 0 for specular materials. So direct light 
//				//sampling with MIS is not valid for specular (delta) mats. The only valid technique is
//				//to sample the BRDF. Since there is only one valid technique, MIS cannot be applied 
//				//(ie light pdf = 0)
//				real pdfLight = (bsdfInfos.sampledType & BSDF::DELTA) ? 0. : lightCaught->pdf(intersection.myPoint, lightInfos);
//				real weight = powerHeuristic(bsdfInfos.pdf, pdfLight);
//
//				Color tr = myMedium.transmittance(Ray(reflected.myOrigin, reflected.direction(), 0., toLightInter.t));
//
//				//Color radianceLight = lightCaught->le(-reflected.direction(), toLightInter.myShadingGeometry.myN);
//				radiance += throughput * radianceLight * bsdfValue * tr * weight /** weightMedium*/;
//				//std::cout << depth << " " << radiance << " " << throughput << " " << radianceLight << " " << bsdfValue << " " << weight << std::endl;
//				if (radiance.isNan())
//					std::cout << "nan 2 " << throughput << " " << radianceLight << " " << bsdfValue << " " << weight << std::endl;
//
//				if (lightCaught == scene.getEnvironmentLight())
//					//noIntersection = true;
//					return radiance;
//			}
//
//			throughput *= bsdfValue;
//			eta *= bsdfInfos.relativeEta;
//
//		}
//
//		if (depth >= 0)
//		{
//
//			real stopVal = 0.9;//std::min(0.9, throughput.luminance() * eta * eta);
//			if (sampler->getNextSample1D() < stopVal)
//			{
//				throughput /= stopVal;
//			}
//			else
//			{
//				//Light::ptr tmp = scene.getEnvironmentLight();
//				//if (tmp == nullptr)
//				//	return radiance;
//
//				//radianceLight = tmp->le(reflected.direction());
//				//radiance += throughput * radianceLight;
//				//const Scene::LightVector& lights = scene.getLights();
//				//Light::ptr envLight = scene.getEnvironmentLight();
//
//				//for (Light::ptr light : lights)
//				//{
//				//	if (light == envLight)
//				//		radiance += throughput * light->le(ray.direction());
//				//	else
//				//		radiance += throughput * light->le(-ray.direction());
//				//}
//				return radiance;
//			}
//		}
//		intersection = toLightInter;
//		ray = reflected;
//
//		depth++;
//	}
//
//
//	return radiance;
//}
//
//RT_REGISTER_TYPE(VolPathTracing, Integrator)






//New direct-phase direct sampling

//VolPathTracing::VolPathTracing(const Parameters& params)
////:myMedium(Parameters())
//{
//	std::string sStrategy = params.getString("strategy", lightStrategy::STRING[LightSamplingStrategy::ONE_LIGHT_UNIFORM]);
//	myStrategy = myStrategiesByName[sStrategy];
//
//	myMaxDepth = params.getInt("maxDepth", 1000);
//}
//
//
//VolPathTracing::~VolPathTracing()
//{
//}
//
//Color VolPathTracing::li(Scene & scene, Sampler::ptr sampler, const Ray & _ray)
//{
//	Color radiance;
//	const real proba = 0.8;
//	Color throughput(1.);
//	real eta = 1.;
//	int depth = 0;
//	Ray ray(_ray);
//	bool shadowCaught = false;
//	Intersection intersection;
//	if (!scene.computeIntersection(ray, intersection))
//	{
//		//Light::ptr envLight = scene.getEnvironmentLight();
//		//if (envLight != nullptr)
//		//{
//		//	radiance += envLight->le(ray.direction());
//		//}
//
//		//return radiance;
//	}
//	bool noIntersection = false;
//
//	//Go to world coord to "orientate" the hemisphere
//	bool fromMedia = false;
//
//	Medium::ptr medium = scene.getCameraMedium();
//
//	while (depth < myMaxDepth)
//	{
//		real t;
//		Color weightMedium;
//		Ray mediumRay(ray);
//		//ray.myMaxT = intersection.t;
//
//		if (medium && medium->sampleDistance(Ray(ray.myOrigin, ray.direction(), ray.myMinT, intersection.t), sampler->getNextSample2D(), t, weightMedium))
//		{
//			throughput *= weightMedium;
//			//std::cout << weightMedium << std::endl;
//			fromMedia = true;
//
//			//Change origin of the ray
//			//intersection.t = t;
//			intersection.myPoint = ray.getPointAt(t);
//
//			/////////////////////////////////////////////
//			//Light sampling
//			LightSamplingInfos lightInfos;
//			Color value = sampleAttenuatedLightDirect(intersection.myPoint, sampler->getNextSample2D(), scene, lightInfos, medium);
//
//			if (!value.isZero())
//			{
//				PhaseFunctionSamplingInfos phaseInfos(lightInfos.interToLight, -ray.direction());
//
//				radiance += throughput * value * medium->evalPF(phaseInfos);
//
//				if (radiance.isNan())
//					std::cout << "nan 1 " << std::endl;
//			}
//
//			/////////////////////////////////////////////
//			//Phase sampling
//			PhaseFunctionSamplingInfos pfInfos(-ray.direction());
//			real pfWeight = medium->samplePF(pfInfos, sampler->getNextSample2D());
//
//			ray.myOrigin = intersection.myPoint;
//			ray.direction(pfInfos.wo);
//
//			throughput *= pfWeight;
//
//			scene.computeIntersection(ray, intersection);
//			//{
//			//	Light::ptr envLight = scene.getEnvironmentLight();
//			//	if (envLight != nullptr)
//			//	{
//			//		radiance += throughput * envLight->le(ray.direction()); //tr ?
//			//	}
//
//			//	return radiance;
//			//}
//		}
//		else
//		{
//			fromMedia = false;
//			BSDF::ptr bsdf = intersection.myPrimitive->getBSDF();
//
//			if (bsdf == nullptr)
//			{
//				ray = Ray(intersection.myPoint, ray.direction());
//				if (intersection.isMediumTransition())
//					medium = intersection.getMedium(ray);
//				continue;
//			}
//
//			if (medium)
//				throughput *= weightMedium;
//
//			//If we have intersected a light, add the radiance
//			if ((depth == 0 && intersection.myPrimitive->isLight()))// || (fromMedia && intersection.myPrimitive->isLight()))
//				radiance += throughput * intersection.myPrimitive->le(-ray.direction(), intersection.myShadingGeometry.myN); //tr ?
//
//																															 //Light sampling strategy
//			LightSamplingInfos lightInfos;
//			Color value = sampleAttenuatedLightDirect(intersection.myPoint, sampler->getNextSample2D(), scene, lightInfos, medium);
//			if (!value.isZero())
//			{
//				Vector3d localWi = intersection.toLocal(lightInfos.interToLight);
//				Vector3d localWo = intersection.toLocal(-ray.direction());
//				real cosTheta = DifferentialGeometry::cosTheta(localWi);
//
//				BSDFSamplingInfos bsdfInfos(intersection, localWi, localWo);
//				bsdfInfos.uv = intersection.myUv;
//
//				//Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))
//
//
//				radiance += throughput * value * bsdf->eval(bsdfInfos) * cosTheta;
//				if (radiance.isNan())
//					std::cout << "nan 1 " << std::endl;
//			}
//
//			//BSDF sampling strategy
//			Vector3d localWi = intersection.toLocal(-ray.direction());
//			BSDFSamplingInfos bsdfInfos(intersection, localWi);
//			bsdfInfos.uv = intersection.myUv; //
//			bsdfInfos.shadowCaught = shadowCaught;
//			Color bsdfValue = bsdf->sample(bsdfInfos, sampler->getNextSample2D());
//
//			if (bsdfValue.isZero())
//				break;
//
//			//Intersection toLightInter;
//			//Go to world coord to "orientate" the hemisphere
//			ray = Ray(intersection.myPoint, intersection.toWorld(bsdfInfos.wo));
//
//			//if we change the medium
//			if (intersection.isMediumTransition())
//				medium = intersection.getMedium(ray);
//
//			scene.computeIntersection(ray, intersection);
//
//			throughput *= bsdfValue;
//			eta *= bsdfInfos.relativeEta;
//			depth++;
//		}
//
//		if (depth >= 0)
//		{
//
//			real stopVal = std::min(0.9, throughput.luminance() * eta * eta);
//			if (sampler->getNextSample1D() < stopVal)
//			{
//				throughput /= stopVal;
//			}
//			else
//			{
//				return radiance;
//			}
//		}
//
//		
//	}
//
//
//	return radiance;
//}
//
//RT_REGISTER_TYPE(VolPathTracing, Integrator)








//Medium sampling only

//#include "VolPathTracing.h"
//
//
//
//#include "Parameters.h"
//#include "Intersection.h"
//#include "Scene.h"
//
//
//VolPathTracing::VolPathTracing(const Parameters& params)
//	:myMedium(Parameters())
//{
//	std::string sStrategy = params.getString("strategy", lightStrategy::STRING[LightSamplingStrategy::ONE_LIGHT_UNIFORM]);
//	myStrategy = myStrategiesByName[sStrategy];
//
//	myMaxDepth = params.getInt("maxDepth", 1000);
//}
//
//
//VolPathTracing::~VolPathTracing()
//{
//}
//
//Color VolPathTracing::li(Scene & scene, Sampler::ptr sampler, const Ray & _ray)
//{
//	//std::cout << "START" << std::endl;
//	Color radiance;
//	const real proba = 0.8;
//	Color throughput(1.);
//	real eta = 1.;
//	int depth = 0;
//	Ray ray(_ray);
//	bool shadowCaught = false;
//	Intersection intersection;
//	if (!scene.computeIntersection(ray, intersection))
//	{
//		Light::ptr envLight = scene.getEnvironmentLight();
//		if (envLight != nullptr)
//		{
//			radiance += envLight->le(ray.direction());
//		}
//
//		return radiance;
//	}
//	bool noIntersection = false;
//	//ray.myMaxT = intersection.t;
//
//	Intersection toLightInter;
//	//Go to world coord to "orientate" the hemisphere
//	Ray reflected;
//	bool fromMedia = false;
//
//	while (depth < myMaxDepth)
//	{
//		//if (noIntersection)
//		//{
//		//	Light::ptr envLight = scene.getEnvironmentLight();
//		//	if (envLight != nullptr)
//		//	{
//		//		radiance += throughput * envLight->le(ray.direction());
//		//	}
//
//		//	return radiance;
//		//}
//
//		real t;
//		Color weightMedium;
//		Ray mediumRay(ray);
//		//ray.myMaxT = intersection.t;
//
//		//If we have intersected a light, add the radiance
//		if ((depth == 0 && intersection.myPrimitive->isLight()) || (fromMedia && intersection.myPrimitive->isLight()))
//			radiance += throughput * intersection.myPrimitive->le(-ray.direction(), intersection.myShadingGeometry.myN);
//
//		if (myMedium.sampleDistance(Ray(ray.myOrigin, ray.direction(), ray.myMinT, intersection.t), sampler->getNextSample2D(), t, weightMedium))
//		{
//			fromMedia = true;
//			throughput *= weightMedium;
//			//LIght sampling
//			//LightSamplingInfos lightInfos;
//			//Color value = sampleLightDirect(intersection.myPoint, sampler->getNextSample2D(), scene, lightInfos, myStrategy);
//			//if (!value.isZero())
//			//{
//			//	Vector3d localWi = intersection.toLocal(lightInfos.interToLight);
//			//	Vector3d localWo = intersection.toLocal(-ray.direction());
//			//	real cosTheta = DifferentialGeometry::cosTheta(localWi);
//
//			//	PhaseFunctionSamplingInfos pfInfos;
//			//	pfInfos.wi = lightInfos.interToLight;
//			//	pfInfos.wo = -ray.direction();
//
//			//	real eval = myMedium.evalPF(pfInfos);
//
//			//	//Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))
//
//			//	//Same as specular BSDF : delta lights cannot be intersected during 
//			//	//BSDF sampling. Then, MIS is not applyable and the "weight" is 1
//
//			//	radiance += throughput * value * eval;
//			//	if (radiance.isNan())
//			//		std::cout << "nan 1 " << std::endl;
//			//}
//			//
//
//
//			//ray.myMaxT = t;
//			//Isotropic iso;
//			PhaseFunctionSamplingInfos pfInfos;
//			//pfInfos.wi = ray.direction()
//			real pfWeight = myMedium.samplePF(pfInfos, sampler->getNextSample2D());
//
//			Ray tmpRay(ray);
//			intersection.myPoint = ray.getPointAt(t);
//			tmpRay.myOrigin = intersection.myPoint;
//			tmpRay.direction(pfInfos.wo);
//			ray = tmpRay;
//
//			//return pfWeight * weightMedium * li(scene, sampler, tmpRay);
//			//radiance += throughput * pfWeight /** weightMedium*/;
//
//			throughput *= /*weightMedium **/ pfWeight;
//
//			if (!scene.computeIntersection(ray, intersection))
//			{
//				Light::ptr envLight = scene.getEnvironmentLight();
//				if (envLight != nullptr)
//				{
//					radiance += throughput * envLight->le(ray.direction());
//				}
//
//				return radiance;
//			}
//			else
//			{
//				//If we caught a light
//				if (intersection.myPrimitive->isLight())
//				{
//					Light::ptr lightCaught = intersection.myPrimitive->getLight();
//					Color radianceLight = lightCaught->le(-ray.direction(), intersection.myShadingGeometry.myN);
//					radiance += throughput * radianceLight;
//				}
//			}
//
//			//dirty
//			toLightInter = intersection;
//			reflected = ray;
//		}
//		else
//		{
//			fromMedia = false;
//			BSDF::ptr bsdf = intersection.myPrimitive->getBSDF();
//
//			if (bsdf == nullptr)
//				return radiance;
//
//
//
//			//Light sampling strategy
//			if (myStrategy == LightSamplingStrategy::ALL_LIGHT)
//			{
//				LightSamplingInfos lightInfos;
//				const Scene::LightVector& lights = scene.getLights();
//				for (unsigned int i = 0; i < lights.size(); i++)
//				{
//					Color value = sampleOneLight(lights[i], intersection.myPoint, sampler->getNextSample2D(), scene, lightInfos);
//					if (!value.isZero())
//					{
//
//						Vector3d localWi = intersection.toLocal(lightInfos.interToLight);
//						Vector3d localWo = intersection.toLocal(-ray.direction());
//						real cosTheta = DifferentialGeometry::cosTheta(localWi);
//
//						BSDFSamplingInfos bsdfInfos(intersection, localWi, localWo);
//						bsdfInfos.uv = intersection.myUv; //
//
//														  //Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))
//
//														  //Same as specular BSDF : delta lights cannot be intersected during 
//														  //BSDF sampling. Then, MIS is not applyable and the "weight" is 1
//						real bsdfPDF = lights[i]->isDelta() ? 0. : bsdf->pdf(bsdfInfos);
//						//Add the MIS heuristic
//						real weight = powerHeuristic(lightInfos.pdf, bsdfPDF);
//
//						radiance += throughput * value * bsdf->eval(bsdfInfos) * cosTheta * weight;
//
//						if (DifferentialGeometry::cosTheta(localWi) <= 0.)
//							shadowCaught = true;
//						else
//							shadowCaught = false;
//					}
//					else
//					{
//						shadowCaught = true;
//					}
//				}
//			}
//			else
//			{
//				throughput *= weightMedium;
//
//				LightSamplingInfos lightInfos;
//				Color value = sampleLightDirect(intersection.myPoint, sampler->getNextSample2D(), scene, lightInfos, myStrategy);
//				if (!value.isZero())
//				{
//					Vector3d localWi = intersection.toLocal(lightInfos.interToLight);
//					Vector3d localWo = intersection.toLocal(-ray.direction());
//					real cosTheta = DifferentialGeometry::cosTheta(localWi);
//
//					BSDFSamplingInfos bsdfInfos(intersection, localWi, localWo);
//					bsdfInfos.uv = intersection.myUv;
//
//					//Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))
//
//					//Same as specular BSDF : delta lights cannot be intersected during 
//					//BSDF sampling. Then, MIS is not applyable and the "weight" is 1
//					real bsdfPDF = lightInfos.light->isDelta() ? 0. : bsdf->pdf(bsdfInfos);
//					//Add the MIS heuristic
//					real weight = powerHeuristic(lightInfos.pdf, bsdfPDF);
//
//					radiance += throughput * value * bsdf->eval(bsdfInfos) * cosTheta * weight;
//					if (radiance.isNan())
//						std::cout << "nan 1 " << std::endl;
//
//					if (DifferentialGeometry::cosTheta(localWi) <= 0.)
//						shadowCaught = true;
//					else
//						shadowCaught = false;
//				}
//				else
//				{
//					shadowCaught = true;
//				}
//			}
//			//std::cout << shadowCaught << std::endl;
//			//BSDF sampling strategy
//			Vector3d localWi = intersection.toLocal(-ray.direction());
//			BSDFSamplingInfos bsdfInfos(intersection, localWi);
//			bsdfInfos.uv = intersection.myUv; //
//			bsdfInfos.shadowCaught = shadowCaught;
//			Color bsdfValue = bsdf->sample(bsdfInfos, sampler->getNextSample2D());
//
//			if (bsdfValue.isZero())
//				break;
//
//			//Intersection toLightInter;
//			//Go to world coord to "orientate" the hemisphere
//			reflected = Ray(intersection.myPoint, intersection.toWorld(bsdfInfos.wo));//, ray.myMinT, ray.myMaxT); //min, max, ok ?
//																					//std::cout << depth << " " << intersection.myPoint << " " << ray.direction() << " " << reflected.direction() << std::endl;
//			Light::ptr lightCaught;
//			Color radianceLight;
//			//Trace a ray from the inter point to the bsdf sampled direction
//			if (scene.computeIntersection(reflected, toLightInter))
//			{
//				//If we caught a light
//				if (toLightInter.myPrimitive->isLight())
//				{
//					lightCaught = toLightInter.myPrimitive->getLight();
//					radianceLight = lightCaught->le(-reflected.direction(), toLightInter.myShadingGeometry.myN);
//				}
//			}
//			else
//			{
//				lightCaught = scene.getEnvironmentLight();
//				if (lightCaught == nullptr)
//					return radiance;
//
//				radianceLight = lightCaught->le(reflected.direction());
//			}
//
//			if (lightCaught != nullptr)
//			{
//				LightSamplingInfos lightInfos;
//				lightInfos.normal = toLightInter.myShadingGeometry.myN; //to change
//				lightInfos.sampledPoint = toLightInter.myPoint; //to change
//				lightInfos.interToLight = reflected.direction();
//				//Light::ptr light = toLightInter.myPrimitive->getLight();
//
//				//pbrt book p 750 :  light'pdf is 0 if BSDF is specular because there is no chance of 
//				//the light sampling the specular direction
//				//Or, better : ompf2.com : MIS Question. BRDF is 0 for specular materials. So direct light 
//				//sampling with MIS is not valid for specular (delta) mats. The only valid technique is
//				//to sample the BRDF. Since there is only one valid technique, MIS cannot be applied 
//				//(ie light pdf = 0)
//				real pdfLight = (bsdfInfos.sampledType & BSDF::DELTA) ? 0. : lightCaught->pdf(intersection.myPoint, lightInfos);
//				real weight = powerHeuristic(bsdfInfos.pdf, pdfLight);
//
//				//Color radianceLight = lightCaught->le(-reflected.direction(), toLightInter.myShadingGeometry.myN);
//				radiance += throughput * radianceLight * bsdfValue * weight /** weightMedium*/;
//				//std::cout << depth << " " << radiance << " " << throughput << " " << radianceLight << " " << bsdfValue << " " << weight << std::endl;
//				if (radiance.isNan())
//					std::cout << "nan 2 " << throughput << " " << radianceLight << " " << bsdfValue << " " << weight << std::endl;
//
//				if (lightCaught == scene.getEnvironmentLight())
//					//noIntersection = true;
//					return radiance;
//			}
//
//			throughput *= bsdfValue;
//			eta *= bsdfInfos.relativeEta;
//
//		}
//
//		if (depth >= 0)
//		{
//
//			real stopVal = 0.9;//std::min(0.9, throughput.luminance() * eta * eta);
//			if (sampler->getNextSample1D() < stopVal)
//			{
//				throughput /= stopVal;
//			}
//			else
//			{
//				//Light::ptr tmp = scene.getEnvironmentLight();
//				//if (tmp == nullptr)
//				//	return radiance;
//
//				//radianceLight = tmp->le(reflected.direction());
//				//radiance += throughput * radianceLight;
//				//const Scene::LightVector& lights = scene.getLights();
//				//Light::ptr envLight = scene.getEnvironmentLight();
//
//				//for (Light::ptr light : lights)
//				//{
//				//	if (light == envLight)
//				//		radiance += throughput * light->le(ray.direction());
//				//	else
//				//		radiance += throughput * light->le(-ray.direction());
//				//}
//				return radiance;
//			}
//		}
//		intersection = toLightInter;
//		ray = reflected;
//
//		depth++;
//	}
//
//
//	return radiance;
//}
//
//RT_REGISTER_TYPE(VolPathTracing, Integrator)
























//Recursive + medium sampling

//#include "VolPathTracing.h"
//
//#include "BSDF.h"
//#include "Intersection.h"
//#include "ObjectFactoryManager.h"
//#include "Scene.h"
//
//
//VolPathTracing::VolPathTracing(const Parameters& params)
//	:myMedium(Parameters())
//{
//	myStrategy = LightSamplingStrategy::ONE_LIGHT_UNIFORM;// params.getString("strategy", "uniform");
//	myMaxDepth = params.getInt("maxDepth", 1000);
//}
//
//
//VolPathTracing::~VolPathTracing()
//{
//}
//
//Color VolPathTracing::li(Scene & scene, Sampler::ptr sampler, const Ray & ray)
//{
//	Intersection intersection;
//	Color throughput(1.);
//	real eta = 1.;
//	return li(scene, sampler, ray, 0, intersection, throughput, eta);
//}
//
//Color VolPathTracing::li(Scene & scene, Sampler::ptr sampler, const Ray & ray, int depth, const Intersection& inter, Color& throughput, real& eta)
//{
//	Color radiance;
//	const real proba = 0.8;
//
//	Intersection intersection = inter;
//	if (depth == 0 && !scene.computeIntersection(ray, intersection))
//	{
//		Light::ptr envLight = scene.getEnvironmentLight();
//		if (envLight != nullptr)
//		{
//			radiance += envLight->le(ray.direction(), Normal3d());
//		}
//
//		return radiance;
//	}
//
//	BSDF::ptr bsdf = intersection.myPrimitive->getBSDF();
//
//	if (bsdf == nullptr)
//		return radiance;
//
//	//If we have intersected a light, add the radiance
//	if (intersection.myPrimitive->isLight())
//		radiance += intersection.myPrimitive->le(-ray.direction(), intersection.myShadingGeometry.myN);
//
//	real t;
//	Color weightMedium(1.);
//	Ray mediumRay(ray);
//	//ray.myMaxT = intersection.t;
//	bool fromMedia = false;
//
//	//If we have intersected a light, add the radiance
//	if ((depth == 0 && intersection.myPrimitive->isLight()) || (fromMedia && intersection.myPrimitive->isLight()))
//		radiance += throughput * intersection.myPrimitive->le(-ray.direction(), intersection.myShadingGeometry.myN);
//
//	if (myMedium.sampleDistance(Ray(ray.myOrigin, ray.direction(), ray.myMinT, intersection.t), sampler->getNextSample2D(), t, weightMedium))
//	{
//		//fromMedia = true;
//
//		//ray.myMaxT = t;
//		//Isotropic iso;
//		PhaseFunctionSamplingInfos pfInfos;
//		//pfInfos.wi = ray.direction()
//		real pfWeight = myMedium.samplePF(pfInfos, sampler->getNextSample2D());
//
//		Ray tmpRay(ray);
//		intersection.myPoint = ray.getPointAt(t);
//		tmpRay.myOrigin = intersection.myPoint;
//		tmpRay.direction(pfInfos.wo);
//
//		//return pfWeight * weightMedium * li(scene, sampler, tmpRay);
//		if (depth < myMaxDepth)
//		{
//			//depth++;
//			real stopVal = 0.8;//std::min(0.9, throughput.luminance() * eta * eta);
//			if (sampler->getNextSample1D() < stopVal)//proba)
//			{
//				throughput /= stopVal;
//				//if(bsdf->pdf(bsdfInfos) == 0.)
//				return pfWeight * weightMedium * li(scene, sampler, tmpRay, ++depth, intersection, throughput, eta) / (stopVal);
//			}
//			else
//			{
//				return radiance;
//			}
//		}
//	}
//	else
//	{
//
//		//Light sampling strategy
//		LightSamplingInfos lightInfos;
//		Color value = sampleLightDirect(intersection.myPoint, sampler->getNextSample2D(), scene, lightInfos, myStrategy);
//		if (!value.isZero())
//		{
//			Vector3d localWi = intersection.toLocal(lightInfos.interToLight);
//			Vector3d localWo = intersection.toLocal(-ray.direction());
//			real cosTheta = DifferentialGeometry::cosTheta(localWi);
//
//			BSDFSamplingInfos bsdfInfos(intersection, localWi, localWo);
//			bsdfInfos.uv = intersection.myUv;
//
//			//Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))
//
//			//Same as specular BSDF : delta lights cannot be intersected during 
//			//BSDF sampling. Then, MIS is not applyable and the "weight" is 1
//			real bsdfPDF = lightInfos.light->isDelta() ? 0. : bsdf->pdf(bsdfInfos);
//			//Add the MIS heuristic
//			real weight = powerHeuristic(lightInfos.pdf, bsdfPDF);//bsdf->pdf(bsdfInfos));
//
//			radiance += weightMedium * value * bsdf->eval(bsdfInfos) * cosTheta * weight;
//			if (radiance.isNan())
//				std::cout << "nan 1 " << std::endl;
//		}
//
//
//		//BSDF sampling strategy
//		Vector3d localWi = intersection.toLocal(-ray.direction());
//		BSDFSamplingInfos bsdfInfos(intersection, localWi);
//		bsdfInfos.uv = intersection.myUv; //
//		Color bsdfValue = bsdf->sample(bsdfInfos, sampler->getNextSample2D());
//		if (bsdfValue.isZero())
//			return radiance;
//
//		Intersection toLightInter;
//		//Go to world coord to "orientate" the hemisphere
//		Ray reflected(intersection.myPoint, intersection.toWorld(bsdfInfos.wo));//, ray.myMinT, ray.myMaxT); //min, max, ok ?
//
//																				//Trace a ray from the inter point to the bsdf sampled direction
//		if (scene.computeIntersection(reflected, toLightInter))
//		{
//			//If we caught a light
//			if (toLightInter.myPrimitive->isLight())
//			{
//				LightSamplingInfos lightInfos;
//				lightInfos.normal = toLightInter.myShadingGeometry.myN;
//				lightInfos.sampledPoint = toLightInter.myPoint; //and wi ??????????
//				Light::ptr light = toLightInter.myPrimitive->getLight();
//
//				//pbrt book p 750 :  light'pdf is 0 is BSDF is specular because there is no chance of 
//				//the light sampling the specular direction
//				//Or, better : ompf2.com : MIS Question. BRDF is 0 for specular materials. So direct light 
//				//sampling with MIS is not valid for specular (delta) mats. The only valid technique is
//				//to sample the BRDF. Since there is only one valid technique, MIS cannot be applied 
//				//(ie light pdf = 0)
//				real pdfLight = (bsdfInfos.sampledType & BSDF::DELTA) ? 0. : light->pdf(intersection.myPoint, lightInfos);
//				real weight = powerHeuristic(bsdfInfos.pdf, pdfLight);
//
//				Color radianceLight = toLightInter.myPrimitive->le(-reflected.direction(), toLightInter.myShadingGeometry.myN);
//				radiance += weightMedium * radianceLight *bsdfValue * weight;
//				if (radiance.isNan())
//					std::cout << "nan 2 " << std::endl;
//			}
//		}
//		else
//		{
//			LightSamplingInfos lightInfos;
//			lightInfos.interToLight = reflected.direction(); //check
//			Light::ptr envLight = scene.getEnvironmentLight();
//			if (envLight != nullptr)
//			{
//				Color lightValue = envLight->le(reflected.direction(), Normal3d());
//				real pdfLight = (bsdfInfos.sampledType & BSDF::DELTA) ? 0. : envLight->pdf(intersection.myPoint, lightInfos);
//				real weight = powerHeuristic(bsdfInfos.pdf, pdfLight); //check pdf envlight = 0
//				if (!std::isnan(weight))
//					radiance += weightMedium * lightValue *bsdfValue * weight;
//			}
//
//			return radiance;
//		}
//
//		if (depth < myMaxDepth)
//		{
//			depth++;
//			real stopVal = 0.8;//std::min(0.9, throughput.luminance() * eta * eta);
//			if (sampler->getNextSample1D() < stopVal)//proba)
//			{
//				throughput /= stopVal;
//				//if(bsdf->pdf(bsdfInfos) == 0.)
//				radiance += weightMedium * bsdfValue * li(scene, sampler, reflected, depth, toLightInter, throughput, eta) / (stopVal);
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
//
//RT_REGISTER_TYPE(VolPathTracing, Integrator)