#pragma once

#include "Geometry.h"

#include <map>

//Macro to associate strings and enum
#define MYLIST(x)       \
x(BECKMANN, "beckmann") \
x(PHONG, "phong") \
x(GGX, "ggx") \
x(END, "null") \

#define USE_FIRST_ELEMENT(x, y)  x,
#define USE_SECOND_ELEMENT(x, y) y,

class MicrofacetDistribution
{
public:
	enum DistributionType {
		MYLIST(USE_FIRST_ELEMENT)
	};

public:
	MicrofacetDistribution();
	MicrofacetDistribution(const std::string& type);
	~MicrofacetDistribution();

	Normal3d sampleNormal(const Point2d& sample, double alpha);

	double D(const Vector3d& halfVector, double alpha);

	double G(const Vector3d& wi, const Vector3d& wo, const Vector3d& wh, double alpha);

	double shadowingTermG1Beckmann(const Vector3d& v, const Vector3d& m, double alpha);

	double shadowingTermG1GGX(const Vector3d& v, const Vector3d& m, double alpha);

	double shadowingTermG1Phong(const Vector3d& v, const Vector3d& m, double alpha);

protected:
	DistributionType myType;

	std::map<std::string, DistributionType> myStrategiesByName;
};

namespace microfacetDistributionString
{
	static const char *STRING[] =
	{
		MYLIST(USE_SECOND_ELEMENT)
	};
}

#undef MYLIST
#undef USE_FIRST_ELEMENT
#undef USE_SECOND_ELEMENT