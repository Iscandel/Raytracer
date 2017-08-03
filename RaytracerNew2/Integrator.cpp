#include "Integrator.h"
#include "Intersection.h"
#include "Scene.h"

Integrator::Integrator()
{
	for (unsigned int i = 0; i < LightSamplingStrategy::END; i++)
	{
		myStrategiesByName[lightStrategy::STRING[i]] = (LightSamplingStrategy) i;
	}
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

Color Integrator::evalTransmittance(const Scene& scene, const Ray& ray, Medium::ptr medium)
{
	Ray trRay(ray);
	Intersection intersection;
	Color tr(1.);
	double distInterLight = ray.myMaxT;

	while (true)
	{
		bool inter = scene.computeIntersection(trRay, intersection, true);
		//If we had a surface intersection
		if (inter && intersection.myPrimitive != nullptr)
			return Color(0.);

		//If we had intersection with bounded media only, decrement transmittance length calculation
		if (inter)
			distInterLight -= intersection.t;

		//if(distInterLight < 0.)
		//	return tr * medium->transmittance(Ray(trRay.myOrigin, trRay.direction(), trRay.myMinT, distInterLight));

		//Update transmittance
		if(medium)
			tr *= medium->transmittance(Ray(trRay.myOrigin, trRay.direction(), trRay.myMinT, inter ? intersection.t : distInterLight));

		//No intersection, leave
		if (!inter)
			return tr;

		//Update the ray
		trRay = Ray(trRay.getPointAt(intersection.t), trRay.direction(), 0., distInterLight);

		if (intersection.isMediumTransition())
			medium = intersection.getMedium(trRay);
	}
	
}

Color Integrator::sampleAttenuatedLightDirect(const Point3d& interPoint, const Point2d& _sample, const Scene& scene, LightSamplingInfos& infos, Medium::ptr medium)
{
	Color value;
	double lightWeight;
	const Scene::LightVector& lights = scene.getLights();
	Point2d sample = _sample;

	int index = myLightWeights.sampleAndReuse(sample.x(), lightWeight);
	Light::ptr light = lights[index];

	//Sample the light
	infos = light->sample(interPoint, sample);
	infos.light = light;

	Ray shadowRay(interPoint, infos.interToLight, tools::EPSILON, infos.distance - tools::EPSILON);
	Intersection tmp;

	//If the light point is visible from the the inter point
	if (infos.pdf > 0 && infos.intensity.luminance() > 0)// && !scene.computeIntersection(shadowRay, tmp, true))
	{
		//Color tr(1.);
		Color tr = evalTransmittance(scene, shadowRay, medium);//medium->transmittance(shadowRay);
		infos.pdf *= lightWeight;
		value = tr * infos.intensity / infos.pdf;
		//value /= lightWeight;
		
	}

	return value;
}

Color Integrator::sampleLightDirect(const Point3d& interPoint, const Point2d& _sample, const Scene& scene, LightSamplingInfos& infos, LightSamplingStrategy strategy)
{
		double lightWeight;
		Light::ptr light;
		const Scene::LightVector& lights = scene.getLights();
		Point2d sample = _sample;

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

		 //To check

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

Color Integrator::sampleOneLight(Light::ptr light, const Point3d & interPoint, const Point2d & sample, const Scene & scene, LightSamplingInfos & lightInfos)
{
	//Sample the light
	lightInfos = light->sample(interPoint, sample);
	lightInfos.light = light;

	Ray shadowRay(interPoint, lightInfos.interToLight, tools::EPSILON, lightInfos.distance - tools::EPSILON);
	Intersection tmp;

	//If the light point is visible from the the inter point
	if (lightInfos.pdf > 0 && lightInfos.intensity.luminance() > 0 && !scene.computeIntersection(shadowRay, tmp, true))
	{
		return lightInfos.intensity / lightInfos.pdf;
	}

	return Color();
}

