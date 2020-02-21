#include "Integrator.h"
#include "core/Intersection.h"
#include "core/Mapping.h"
#include "core/Math.h"
#include "core/Scene.h"

Integrator::Integrator(const Parameters& params)
{
	for (unsigned int i = 0; i < LightSamplingStrategy::END; i++)
	{
		myStrategiesByName[lightStrategy::STRING[i]] = (LightSamplingStrategy) i;
	}

	std::string sStrategy = params.getString("strategy", lightStrategy::STRING[LightSamplingStrategy::ONE_LIGHT_UNIFORM]);
	myLightStrategy = myStrategiesByName[sStrategy];
}

Integrator::~Integrator()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void Integrator::initialize(Scene & scene)
{
	const Scene::LightVector& lights = scene.getLights();
	real pdfUniform = (real)1. / lights.size();

	for (Light::ptr light : lights)
	{
		if(myLightStrategy == LightSamplingStrategy::ONE_LIGHT_WEIGHTED)
			myLightWeights.add(light->power().luminance());
		else
			myLightWeights.add(pdfUniform);
	}

	myLightWeights.normalize();
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color Integrator::irradiance(Scene& scene, int lightSamples, const Intersection& inter,
		Medium::ptr medium,  Sampler::ptr& sampler, bool catchIndirect)
{
	Color irrLight;
	static int access = 0;
	for (int i = 0; i < lightSamples; i++)
	{
		sampler->startPixel();
		LightSamplingInfos infos;
		Color direct = sampleAttenuatedLightDirect(inter.myPoint, sampler, scene, infos, medium);
		if (!direct.isZero())
		{
			real cosTheta = infos.interToLight.dot(inter.myShadingGeometry.myN);
			if (cosTheta > 0)
				irrLight += direct * cosTheta;
		}

		if (catchIndirect)
		{
			Vector3d dir = inter.toWorld(Mapping::squareToCosineWeightedHemisphere(sampler->getNextSample2D()));
			Integrator::RadianceType::ERadianceType type = Integrator::RadianceType::INDIRECT_RADIANCE;
			Ray ray(inter.myPoint, dir);
			//Indirect irradiance is E = integral(Li (n.w) dw) = Li / pdf * (n.w)
			//pdf is a weighted cosine -> cos / pi
			//so E = Li * pi
			access++;
			irrLight += li(scene, sampler, ray, type) * math::PI;
		}

		if (irrLight.minCoeff() < -1.5)
			std::cout << "neg " << i << " " <<access << " " << irrLight;
		sampler->advance();
	}

	return irrLight / (real)lightSamples;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color Integrator::evalTransmittance(const Scene& scene, const Ray& ray, Medium::ptr medium, Sampler::ptr sampler)
{
	Ray trRay(ray);
	Intersection intersection;
	Color tr(1.);
	real distInterLight = ray.myMaxT;
	bool firstPass = true;

	while (true)
	{
		bool inter = scene.computeIntersection(trRay, intersection);
		//If we had a surface intersection
		if (inter && (intersection.myPrimitive->getBSDF() != nullptr && !intersection.myPrimitive->getBSDF()->isTransparent()))
			return Color(0.);

		//If we had an intersection with bounded media only, decrement transmittance length calculation
		if (inter)
			distInterLight -= intersection.t;

		//if(distInterLight < 0.)
		//	return tr * medium->transmittance(Ray(trRay.myOrigin, trRay.direction(), trRay.myMinT, distInterLight));

		//Update transmittance
		if(medium)
			tr *= medium->transmittance(Ray(trRay.myOrigin, trRay.direction(), firstPass ? trRay.myMinT : 0.f, inter ? intersection.t : distInterLight), sampler);

		firstPass = false;
		//No intersection, leave
		if (!inter || tr.isZero())
			return tr;

		//
		if (intersection.myPrimitive->getBSDF())
		{
			Vector3d localWi = intersection.toLocal(-trRay.direction());
			Vector3d localWo = intersection.toLocal(trRay.direction());
			BSDFSamplingInfos bsdfInfos(intersection, localWi, localWo);
			bsdfInfos.uv = intersection.myUv; //
			bsdfInfos.measure = Measure::DISCRETE;
			tr *= intersection.myPrimitive->getBSDF()->eval(bsdfInfos);
		}

		if (tr.isZero())
			return tr;

		//Update the ray
		trRay = Ray(trRay.getPointAt(intersection.t), trRay.direction(), math::EPSILON, distInterLight);

		if (intersection.isMediumTransition())
			medium = intersection.getMedium(trRay);
	}
	
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color Integrator::sampleAttenuatedLightDirect(const Point3d& interPoint, Sampler::ptr sampler, const Scene& scene, LightSamplingInfos& infos, Medium::ptr medium)
{
	Color value;
	real lightWeight;
	const Scene::LightVector& lights = scene.getLights();
	if (lights.size() == 0)
		return Color();

	Point2d sample = sampler->getNextSample2D();

	int index = myLightWeights.sampleAndReuse(sample.x(), lightWeight);
	Light::ptr light = lights[index];

	//Light::ptr light = lights[(int)(sample.x() * lights.size())];
	//lightWeight = 1.f / lights.size();

	//Sample the light
	infos = light->sample(interPoint, sample);
	infos.light = light;

	Ray shadowRay(interPoint, infos.interToLight, math::EPSILON, infos.distance - math::EPSILON);
	Intersection tmp;

	//If the light point is visible from the the inter point
	if (infos.pdf > 0 && infos.intensity.luminance() > 0)// && !scene.computeIntersection(shadowRay, tmp, true))
	{
		//Color tr(1.);
		Color tr = evalTransmittance(scene, shadowRay, medium, sampler);//medium->transmittance(shadowRay);
		infos.pdf *= lightWeight;
		value = tr * infos.intensity / infos.pdf;
		//value /= lightWeight;
		
	}

	return value;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color Integrator::sampleLightDirect(const Point3d& interPoint, const Point2d& _sample, const Scene& scene, LightSamplingInfos& infos, LightSamplingStrategy strategy)
{
		real lightWeight;
		Light::ptr light;
		const Scene::LightVector& lights = scene.getLights();
		if (lights.size() == 0)
			return Color();
		Point2d sample = _sample;

		//if (strategy == LightSamplingStrategy::ONE_LIGHT_WEIGHTED)
		//{
			int index = myLightWeights.sampleAndReuse(sample.x(), lightWeight);
			light = lights[index];
		//}
		//else
		//{
		//	light = lights[(int)(sample.x() * lights.size())];
		//	lightWeight = 1.f / lights.size();
		//}

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

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color Integrator::sampleOneLight(Light::ptr light, const Point3d & interPoint, const Point2d & sample, const Scene & scene, LightSamplingInfos & lightInfos)
{
	//Sample the light
	lightInfos = light->sample(interPoint, sample);
	lightInfos.light = light;

	Ray shadowRay(interPoint, lightInfos.interToLight, math::EPSILON, lightInfos.distance - math::EPSILON);
	Intersection tmp;

	//If the light point is visible from the the inter point
	if (lightInfos.pdf > 0 && lightInfos.intensity.luminance() > 0 && !scene.computeIntersection(shadowRay, tmp, true))
	{
		return lightInfos.intensity / lightInfos.pdf;
	}

	return Color();
}

