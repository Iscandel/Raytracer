#pragma once
#include "Light.h"

#include "core/Geometry.h"
#include "core/Transform.h"
#include "core/Math.h"

class Parameters;

class SpotLight :
	public Light
{
public:
	SpotLight(const Parameters& params);
	~SpotLight();

	Color power() const override
	{
		return 2 * math::PI * myIntensity * (1.f - 0.5f * (myCosFalloffStart + myCosTotalWidth));
	}

	LightSamplingInfos sample(const Point3d& pFrom, const Point2d& sample) override;

	real falloff(const Vector3d& dir);

	real pdf(const Point3d&, const LightSamplingInfos&) override;

	//Shouldn't be called I think
	Color le(const Vector3d&, const Normal3d&) const override { return Color(); }

	bool isDelta() const override { return true; }

protected:
	Color myIntensity;
	real myCosFalloffStart;
	real myCosTotalWidth;
	Transform::ptr myLightToWorld;
	Point3d myCenter;
};

