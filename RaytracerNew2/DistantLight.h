#pragma once
#include "Light.h"

#include "Transform.h"

#include <memory>

class ISampledShape;
class Parameters;

///////////////////////////////////////////////////////////////////////////////
/// \brief General class for surface based lights.
///////////////////////////////////////////////////////////////////////////////
class DistantLight : public Light
{
public:
	typedef std::shared_ptr<DistantLight> ptr;

public:
	DistantLight(const Parameters& params);
	virtual ~DistantLight();

	virtual LightSamplingInfos sample(const Point3d& pFrom, const Point2d& sample) override;

	virtual double pdf(const Point3d& pFrom, const LightSamplingInfos& infos) override;

	virtual Color power() const override;

	virtual Color le(const Vector3d& direction, const Normal3d& normal = Normal3d()) const override;

	void initialize(const Scene& scene) override;

protected:
	Color myRadiance;
	Transform::ptr myLightToWorld;
	double myAngle;

	//Bounding sphere
	double mySphereRadius;
	Point3d myCenter;
};



