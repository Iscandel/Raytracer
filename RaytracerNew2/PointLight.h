#pragma once
#include "Light.h"

#include "Parameters.h"

class PointLight : public Light
{
public:
	PointLight(const Parameters& params);
	~PointLight();

	Color power() const override
	{
		return 4 * tools::PI * myIntensity;
	}

	LightSamplingInfos sample(const Point3d& pFrom, const Point2d& sample) override;

	double pdf(const Point3d& pFrom, const LightSamplingInfos& infos) override
	{
		return 0.;
	}

	//Shouldn't be called I think
	Color le(const Vector3d& direction, const Normal3d& normal = Normal3d()) const
	{
		return Color();
	}

protected:
	Point3d myPosition;
	Color myIntensity;
};

