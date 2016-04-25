#include "Integrator.h"
#include "Intersection.h"
#include "Scene.h"

Integrator::Integrator()
{
}


Integrator::~Integrator()
{
}

void Integrator::initialize(Scene & scene)
{
	const Scene::LightVector& lights = scene.getLights();

	for (Light::ptr light : lights)
	{
		myLightWeights.add(light->power().luminance());
	}

	myLightWeights.normalize();
}


Color Integrator::sampleLightDirect(const Point3d& interPoint, const Point2d& _sample, const Scene& scene, LightSamplingInfos& infos, LightSamplingStrategy strategy)
{
		double lightWeight;
		Light::ptr light;
		const Scene::LightVector& lights = scene.getLights();
		Point2d sample = sample;

		if (strategy == LightSamplingStrategy::ONE_LIGHT_WEIGHTED)
		{
			int index = myLightWeights.sampleAndReuse(sample.x(), lightWeight);
			light = lights[index];
		}
		else
		{
			light = lights[(int)(sample.x() * lights.size())];
			lightWeight = 1. / lights.size();
		}

		Color value = sampleOneLight(light, interPoint, sample, scene, infos);
		infos.pdf *= lightWeight;

		value /= lightWeight;

		////Sample the light
		//LightSamplingInfos lightInfos = light->sample(interPoint, sample);
		//lightInfos.pdf *= lightWeight;

		//Vector3d interToLight = lightInfos.samplingPoint - interPoint;
		//Ray shadowRay(interPoint, lightInfos.wi, tools::EPSILON, interToLight.norm() - tools::EPSILON);
		//Intersection tmp;

		////If the light point is visible from the the inter point
		//if (lightInfos.pdf > 0 && lightInfos.intensity.luminance() > 0 && !scene.computeIntersection(shadowRay, tmp, true))
		//{
		//	return lightInfos.intensity / lightInfos.pdf;
		//}

		return value;
	}

Color Integrator::sampleOneLight(Light::ptr light, const Point3d & interPoint, const Point2d & sample, const Scene & scene, LightSamplingInfos & infos)
{
	//Sample the light
	LightSamplingInfos lightInfos = light->sample(interPoint, sample);

	Ray shadowRay(interPoint, lightInfos.interToLight, tools::EPSILON, lightInfos.distance - tools::EPSILON);
	Intersection tmp;

	//If the light point is visible from the the inter point
	if (lightInfos.pdf > 0 && lightInfos.intensity.luminance() > 0 && !scene.computeIntersection(shadowRay, tmp, true))
	{
		return lightInfos.intensity / lightInfos.pdf;
	}

	return Color();
}

