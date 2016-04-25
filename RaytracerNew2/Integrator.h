#pragma once

#include "CDF.h"
#include "Color.h"
#include "Light.h"
#include "Ray.h"
#include "Sampler.h"

#include <memory>
class Scene;
class Intersection;

class Integrator
{
public:
	typedef std::shared_ptr<Integrator> ptr;
public:
	enum LightSamplingStrategy
	{
		ALL_LIGHT,
		ONE_LIGHT_UNIFORM,
		ONE_LIGHT_WEIGHTED
	};
public:
	Integrator();
	~Integrator();

	void initialize(Scene& scene);

	///Chooses a light in the scene according to the given strategy and then samples it.
	Color sampleLightDirect(const Point3d& interPoint, const Point2d& sample, 
		const Scene& scene, LightSamplingInfos& infos, LightSamplingStrategy strategy = ONE_LIGHT_UNIFORM);
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
};

