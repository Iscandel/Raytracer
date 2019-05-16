#pragma once

#include "Color.h"
#include "Common.h"
#include "Geometry.h"
#include "RaytracerObject.h"
#include "Sampler.h"
#include "WithSmartPtr.h"

#include <memory>

class Scene;
class Light;

struct LightSamplingInfos
{
	LightSamplingInfos::LightSamplingInfos() {
		measure = Measure::SOLID_ANGLE;
		pdf = 0.;
		distance = 0.;
	}

	///Sampled point on the light
	Point3d sampledPoint;

	///Radiance of the light
	Color intensity;

	///Kind of wi, but not in local coords
	Vector3d interToLight;//wi;

	//Norm of interToLight vector
	real distance;

	///Normal at the sampled point
	Normal3d normal; //useless ?

	///Type of measure performed when sampling the light
	Measure measure;

	///Probability density associated to the sampled point
	real pdf;

	//typename Light::ptr light;
	std::shared_ptr<Light> light;
};

class Light : public RaytracerObject, public WithSmartPtr<Light>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

//public:
//	typedef std::shared_ptr<Light> ptr;

	virtual Color power() const = 0;

	virtual LightSamplingInfos sample(const Point3d& pFrom, const Point2d& sample) = 0;

	virtual real pdf(const Point3d& pFrom, const LightSamplingInfos& infos) = 0;

	virtual Color le(const Vector3d& direction, const Normal3d& normal = Normal3d()) const = 0;

	virtual bool isDelta() const = 0;

	//virtual void initialize(const Scene&) {}
public:
	Light() {}
	//Light(const Point3d& center, const Color& col);
	~Light(void);
};

