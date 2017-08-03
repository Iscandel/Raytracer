#pragma once

#include "CDF.h"
#include "Color.h"
#include "Light.h"
#include "Medium.h"
#include "Ray.h"
#include "Sampler.h"

#include <map>
#include <memory>
class Scene;
class Intersection;

//Macro to associate strings and enum
#define MYLIST(x)       \
x(ALL_LIGHT, "all_lights") \
x(ONE_LIGHT_UNIFORM, "one_light_uniform") \
x(ONE_LIGHT_WEIGHTED, "one_light_weighted") \
x(END, "null") \

#define USE_FIRST_ELEMENT(x, y)  x,
#define USE_SECOND_ELEMENT(x, y) y,

class Integrator
{
public:
	typedef std::shared_ptr<Integrator> ptr;
public:
	enum LightSamplingStrategy
	{
		MYLIST(USE_FIRST_ELEMENT)
	};
public:
	Integrator();
	virtual ~Integrator();

	void initialize(Scene& scene);

	///Chooses a light in the scene according to the given strategy and then samples it.
	Color sampleLightDirect(const Point3d& interPoint, const Point2d& sample, 
		const Scene& scene, LightSamplingInfos& infos, LightSamplingStrategy strategy = ONE_LIGHT_UNIFORM);

	Color sampleAttenuatedLightDirect(const Point3d& interPoint, const Point2d& sample,
		const Scene& scene, LightSamplingInfos& infos, Medium::ptr medium);

	Color evalTransmittance(const Scene& scene, const Ray& ray, Medium::ptr medium);

	virtual Color li(Scene& scene, Sampler::ptr sampler, const Ray& ray) = 0;

	Color sampleOneLight(Light::ptr light, const Point3d& interPoint, const Point2d& sample,
		const Scene& scene, LightSamplingInfos& infos);

	double balanceHeuristic(double pdfFirst, double pdfSec)
	{
		return pdfFirst / (pdfFirst + pdfSec);
	}

	double powerHeuristic(double pdfFirst, double pdfSec)
	{
		double left = pdfFirst * pdfFirst;
		double right = pdfSec * pdfSec;
		return left / (left + right);
	}

protected:
	CDF myLightWeights;

	std::map<std::string, LightSamplingStrategy> myStrategiesByName;
};

namespace lightStrategy
{
	static const char *STRING[] =
	{
		MYLIST(USE_SECOND_ELEMENT)
	};
};


#undef MYLIST
#undef USE_FIRST_ELEMENT
#undef USE_SECOND_ELEMENT
