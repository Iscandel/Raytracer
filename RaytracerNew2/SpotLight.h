#pragma once
#include "Light.h"

#include "Geometry.h"
#include "Transform.h"
#include "Tools.h"

class Parameters;

class SpotLight :
	public Light
{
public:
	SpotLight(const Parameters& params);
	~SpotLight();

	Color power() const override
	{
		return 2 * tools::PI * myIntensity * (1. - 0.5 * (myCosFalloffStart + myCosTotalWidth));
	}

	LightSamplingInfos sample(const Point3d& pFrom, const Point2d& sample) override;

	double falloff(const Vector3d& dir);

	double pdf(const Point3d&, const LightSamplingInfos&) override;

	//Shouldn't be called I think
	Color le(const Vector3d&, const Normal3d&) const override { return Color(); }

	bool isDelta() const override { return true; }

protected:
	Color myIntensity;
	double myCosFalloffStart;
	double myCosTotalWidth;
	Transform::ptr myLightToWorld;
	Point3d myCenter;
};

