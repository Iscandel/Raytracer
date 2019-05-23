#pragma once
#include "Light.h"

#include "core/Transform.h"

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

	virtual real pdf(const Point3d& pFrom, const LightSamplingInfos& infos) override;

	virtual Color power() const override;

	virtual Color le(const Vector3d& direction, const Point3d& hitPoint = Point3d(), const Normal3d& normal = Normal3d()) const override;

	bool isDelta() const override { return false; }

	void initialize(Scene& scene) override;

protected:
	Color myRadiance;
	Transform::ptr myLightToWorld;
	real myAngle;

	//Bounding sphere
	real mySphereRadius;
	Point3d myCenter;
};



