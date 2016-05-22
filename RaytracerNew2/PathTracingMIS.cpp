#include "PathTracingMIS.h"

#include "Parameters.h"
#include "Intersection.h"
#include "Scene.h"


PathTracingMIS::PathTracingMIS(const Parameters& params)
{
	std::string sStrategy = params.getString("strategy", lightStrategy::STRING[LightSamplingStrategy::ONE_LIGHT_UNIFORM]);
	myStrategy = myStrategiesByName[sStrategy];

	myMaxDepth = params.getInt("maxDepth", 1000);
}


PathTracingMIS::~PathTracingMIS()
{
}

Color PathTracingMIS::li(Scene & scene, Sampler::ptr sampler, const Ray & _ray)
{
	Color radiance;
	const double proba = 0.8;
	Color throughput(1.);
	double eta = 1.;
	int depth = 0;
	Ray ray(_ray);

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

	while (depth < myMaxDepth)
	{
		BSDF::ptr bsdf = intersection.myPrimitive->getBSDF();

		if (bsdf == nullptr)
			return radiance;

		//If we have intersected a light, add the radiance
		if (depth == 0 && intersection.myPrimitive->isLight())
			radiance += intersection.myPrimitive->le(-ray.direction(), intersection.myShadingGeometry.myN);

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
					double cosTheta = DifferentialGeometry::cosTheta(localWi);

					BSDFSamplingInfos bsdfInfos(localWi, localWo);
					bsdfInfos.uv = intersection.myUv; //

					//Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))

					//Same as specular BSDF : delta lights cannot be intersected during 
					//BSDF sampling. Then, MIS is not applyable and the "weight" is 1
					double bsdfPDF = lights[i]->isDelta() ? 0. : bsdf->pdf(bsdfInfos);
					//Add the MIS heuristic
					double weight = powerHeuristic(lightInfos.pdf, bsdfPDF);

					radiance += throughput * value * bsdf->eval(bsdfInfos) * cosTheta * weight;
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
				double cosTheta = DifferentialGeometry::cosTheta(localWi);

				BSDFSamplingInfos bsdfInfos(localWi, localWo);
				bsdfInfos.uv = intersection.myUv;

				//Compute the radiance using a MC estimator : (1/N) * (bsdf * (light * cosT) / pdf))

				//Same as specular BSDF : delta lights cannot be intersected during 
				//BSDF sampling. Then, MIS is not applyable and the "weight" is 1
				double bsdfPDF = lightInfos.light->isDelta() ? 0. : bsdf->pdf(bsdfInfos);
				//Add the MIS heuristic
				double weight = powerHeuristic(lightInfos.pdf, bsdfPDF);

				radiance += throughput * value * bsdf->eval(bsdfInfos) * cosTheta * weight;
				if (radiance.isNan())
					std::cout << "nan 1 " << std::endl;
			}
		}

		//BSDF sampling strategy
		Vector3d localWi = intersection.toLocal(-ray.direction());
		BSDFSamplingInfos bsdfInfos(localWi);
		bsdfInfos.uv = intersection.myUv; //
		Color bsdfValue = bsdf->sample(bsdfInfos, sampler->getNextSample2D());
		if (bsdfValue.isZero())
			break;

		Intersection toLightInter;
		//Go to world coord to "orientate" the hemisphere
		Ray reflected(intersection.myPoint, intersection.toWorld(bsdfInfos.wo));//, ray.myMinT, ray.myMaxT); //min, max, ok ?

		Light::ptr lightCaught;
		Color radianceLight;
		//Trace a ray from the inter point to the bsdf sampled direction
		if (scene.computeIntersection(reflected, toLightInter))
		{
			//If we caught a light
			if (toLightInter.myPrimitive->isLight())
			{
				lightCaught = toLightInter.myPrimitive->getLight();
				radianceLight = lightCaught->le(-reflected.direction(), toLightInter.myShadingGeometry.myN);
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

			//pbrt book p 750 :  light'pdf is 0 is BSDF is specular because there is no chance of 
			//the light sampling the specular direction
			//Or, better : ompf2.com : MIS Question. BRDF is 0 for specular materials. So direct light 
			//sampling with MIS is not valid for specular (delta) mats. The only valid technique is
			//to sample the BRDF. Since there is only one valid technique, MIS cannot be applied 
			//(ie light pdf = 0)
			double pdfLight = (bsdfInfos.sampledType & BSDF::DELTA) ? 0. : lightCaught->pdf(intersection.myPoint, lightInfos);
			double weight = powerHeuristic(bsdfInfos.pdf, pdfLight);

			//Color radianceLight = lightCaught->le(-reflected.direction(), toLightInter.myShadingGeometry.myN);
			radiance += throughput * radianceLight * bsdfValue * weight;
			if (radiance.isNan())
				std::cout << "nan 2 " << throughput << " " << radianceLight << " " << bsdfValue << " " << weight << std::endl;

			if (lightCaught == scene.getEnvironmentLight())
				return radiance;
		}

		throughput *= bsdfValue;
		eta *= bsdfInfos.relativeEta;

		if (depth >= 0)
		{
			
			double stopVal = 0.9;//std::min(0.9, throughput.luminance() * eta * eta);
			if (sampler->getNextSample1D() < stopVal)
			{
				throughput /= stopVal;
			}
			else
			{
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