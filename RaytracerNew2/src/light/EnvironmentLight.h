#pragma once
#include "Light.h"

#include "core/CDF.h"
#include "core/Parameters.h"

#include "tools/Array2D.h"

#include <vector>

class EnvironmentLight :
	public Light
{
public:
	EnvironmentLight(const Parameters& params);
	~EnvironmentLight();

	Color power() const;

	LightSamplingInfos sample(const Point3d& pFrom, const Point2d& sample) override;

	real pdf(const Point3d& pFrom, const LightSamplingInfos& infos) override;

	Color le(const Vector3d& direction, const Point3d& hitPoint = Point3d(), const Normal3d& normal = Normal3d()) const override;

	bool isDelta() const override { return false; }

	//Color interp2(const Point2d& xy) const;

	

	void initialize(Scene& scene) override;

	Color getValue(int x, int y, bool scale = true)
	{
		if (scale)
			return (*myArray)(x, y) * myFactor;
		return (*myArray)(x, y);
	}

protected:
	void checkXY(real& x, real& y) const;

protected:
	Transform::ptr myLightToWorld;
	Transform::ptr myWorldToLight;

	real myFactor;

	//Bounding sphere
	real mySphereRadius;
	Point3d myCenter;
	bool myIsProbe;
	
	std::vector<CDF> myRowCDFs;
	CDF myMarginalCDF; //to choose a row (sum of the columns on a row)

	std::shared_ptr<Array2D<Color3>> myArray;

	std::vector<real> myWeight;

	real myOffsetU;
	real myOffsetV;
};

