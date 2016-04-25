#pragma once
#include "Light.h"

#include "CDF.h"
#include "Parameters.h"

#include "Array2D.h"

#include <vector>

class EnvironmentLight :
	public Light
{
public:
	EnvironmentLight(const Parameters& params);
	~EnvironmentLight();

	Color power() const;

	LightSamplingInfos sample(const Point3d& pFrom, const Point2d& sample) override;

	double pdf(const Point3d& pFrom, const LightSamplingInfos& infos) override;

	Color le(const Vector3d& direction, const Normal3d& normal = Normal3d()) const override;

	Color interp2(const Point2d& xy) const;

	std::vector<CDF> myRowCDFs;
	CDF myMarginalCDF; //to choose a row (sum of the columns on a row)

	Array2D<Color> myArray;

	std::vector<double> myWeight;

	void initialize(const Scene& scene) override;

protected:
	Transform::ptr myLightToWorld;
	Transform::ptr myWorldToLight;

	//Bounding sphere
	double mySphereRadius;
	Point3d myCenter;
};

