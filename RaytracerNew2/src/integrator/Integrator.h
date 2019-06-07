#pragma once

#include "core/CDF.h"
#include "core/Color.h"
#include "light/Light.h"
#include "medium/Medium.h"
#include "core/Parameters.h"
#include "core/Ray.h"
#include "core/RaytracerObject.h"
#include "sampler/Sampler.h"

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

class Integrator : public RaytracerObject
{
public:
	typedef std::shared_ptr<Integrator> ptr;
public:
	struct RadianceType
	{
		enum ERadianceType
		{
			EMISSION = 1,
			DIRECT_RADIANCE = 2,
			INDIRECT_RADIANCE = 4,
			SUBSURFACE_RADIANCE = 8,
			VOLUME_RADIANCE = 16,
			NO_EMISSION = DIRECT_RADIANCE | INDIRECT_RADIANCE | SUBSURFACE_RADIANCE | VOLUME_RADIANCE,
			ALL_RADIANCE = EMISSION | NO_EMISSION
		};
	};
public:
	enum LightSamplingStrategy
	{
		MYLIST(USE_FIRST_ELEMENT)
	};
public:
	Integrator(const Parameters& params);
	virtual ~Integrator();

	void initialize(Scene& scene) override;

	Color irradiance(Scene& scene, int lightSamples, const Intersection& inter,
		Medium::ptr medium, Sampler::ptr& sampler, bool catchIndirect = true);

	///Chooses a light in the scene according to the given strategy and then samples it.
	Color sampleLightDirect(const Point3d& interPoint, const Point2d& sample, 
		const Scene& scene, LightSamplingInfos& infos, LightSamplingStrategy strategy = ONE_LIGHT_UNIFORM);

	Color sampleAttenuatedLightDirect(const Point3d& interPoint, Sampler::ptr sampler,
		const Scene& scene, LightSamplingInfos& infos, Medium::ptr medium);

	static Color evalTransmittance(const Scene& scene, const Ray& ray, Medium::ptr medium, Sampler::ptr sampler);

	virtual Color li(Scene& scene, Sampler::ptr sampler, const Ray& ray, RadianceType::ERadianceType radianceType = RadianceType::ALL_RADIANCE) = 0;

	Color sampleOneLight(Light::ptr light, const Point3d& interPoint, const Point2d& sample,
		const Scene& scene, LightSamplingInfos& infos);

	real balanceHeuristic(real pdfFirst, real pdfSec)
	{
		return pdfFirst / (pdfFirst + pdfSec);
	}

	real powerHeuristic(real pdfFirst, real pdfSec)
	{
		real left = pdfFirst * pdfFirst;
		real right = pdfSec * pdfSec;
		return left / (left + right);
	}

protected:
	CDF myLightWeights;

	LightSamplingStrategy myLightStrategy;
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
