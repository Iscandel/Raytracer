#include "MicrofacetDistribution.h"

#include "core/DifferentialGeometry.h"
#include "core/Math.h"


const char *microfacetDistributionString::STRING[];

MicrofacetDistribution::MicrofacetDistribution()
:MicrofacetDistribution(microfacetDistributionString::STRING[0])
{
}

MicrofacetDistribution::MicrofacetDistribution(const std::string& type)
{
	for (unsigned int i = 0; i < DistributionType::END; i++)
	{
		myStrategiesByName[microfacetDistributionString::STRING[i]] = (DistributionType)i;
	}

	myType = myStrategiesByName[type];
}


MicrofacetDistribution::~MicrofacetDistribution()
{
}

Normal3d MicrofacetDistribution::sampleNormal(const Point2d& sample, real alpha)
{
	if (myType == BECKMANN)
	{
		real theta = std::atan(std::sqrt(-alpha * alpha * std::log(1 - sample.x())));
		real phi = 2 * math::PI * sample.y();

		return Normal3d(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta));
	}
	else if (myType == GGX)
	{
		real theta = std::atan(alpha * std::sqrt(sample.x()) / std::sqrt(1.f - sample.x()));
		real phi = 2 * math::PI * sample.y();

		return Normal3d(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta));
	}
	else
	{
		real theta = std::acos(std::pow(sample.x(), 1.f / (alpha + 2)));
		real phi = 2 * math::PI * sample.y();

		return Normal3d(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta));
	}
}

real MicrofacetDistribution::D(const Vector3d & wh, real alpha)
{
	if (myType == BECKMANN)
	{
		if (DifferentialGeometry::cosTheta(wh) <= 0.)
			return 0.;

		real cosThetaH2 = DifferentialGeometry::cosTheta(wh) * DifferentialGeometry::cosTheta(wh);
		real tanThetaH2 = DifferentialGeometry::sinTheta2(wh) / cosThetaH2;
		real alpha2 = alpha * alpha;
		return (math::fastExp(-tanThetaH2 / alpha2)) / (math::PI * alpha2 * cosThetaH2 * cosThetaH2);
	}
	else if (myType == GGX)
	{
		if (DifferentialGeometry::cosTheta(wh) <= 0.)
			return 0.;

		real alpha2 = alpha * alpha;
		real cosThetaH2 = DifferentialGeometry::cosTheta2(wh);
		real tanThetaH2 = DifferentialGeometry::tanTheta2(wh);
		real smallSum = (alpha2 + tanThetaH2);

		return alpha2 / (math::PI * cosThetaH2 * cosThetaH2 * smallSum * smallSum);
	}
	else
	{
		if (DifferentialGeometry::cosTheta(wh) <= 0.)
			return 0.;

		real cosThetaH = DifferentialGeometry::cosTheta(wh);

		return ((alpha + 2.f) / (2 * math::PI)) * std::pow(cosThetaH, alpha);
	}
}

real MicrofacetDistribution::G(const Vector3d & wi, const Vector3d & wo, const Vector3d & wh, real alpha)
{
	if (myType == BECKMANN)
	{
		return shadowingTermG1Beckmann(wi, wh, alpha) * shadowingTermG1Beckmann(wo, wh, alpha);
	}
	else if (myType == GGX)
	{
		return shadowingTermG1GGX(wi, wh, alpha) * shadowingTermG1GGX(wo, wh, alpha);
	}
	else
	{
		return shadowingTermG1Phong(wi, wh, alpha) * shadowingTermG1Phong(wo, wh, alpha);
	}
}

real MicrofacetDistribution::shadowingTermG1Beckmann(const Vector3d& v, const Vector3d& m, real alpha)
{
	if ((v.dot(m)) / (DifferentialGeometry::cosTheta(v)) <= 0)
		return 0.f;

	real tanThetaV = DifferentialGeometry::sinTheta(v) /
		DifferentialGeometry::cosTheta(v);
	
	//if (tanThetaV < 0)
	//	ILogger::log() << "Microfacet distrib: thetaV < 0 .. ?";

	//Perpendicular incidence, no shadowing
	if (tanThetaV == 0.f)
		return 1.f;

	real b = std::abs(1.f / (alpha * tanThetaV));

	if (b < 1.6f)
	{
		return (3.535f * b + 2.181f * b * b) / (1 + 2.276f * b + 2.577f * b * b);
	}
	else
	{
		return 1.f;
	}
}

real MicrofacetDistribution::shadowingTermG1GGX(const Vector3d& v, const Vector3d& m, real alpha)
{
	if ((v.dot(m)) / (DifferentialGeometry::cosTheta(v)) <= 0)
		return 0.;

	real alpha2 = alpha * alpha;
	real tan2 = DifferentialGeometry::tanTheta2(v);
	return 2.f / (1 + std::sqrt(1 + alpha2 * tan2));
}

real MicrofacetDistribution::shadowingTermG1Phong(const Vector3d& v, const Vector3d& m, real alpha)
{
	if ((v.dot(m)) / (DifferentialGeometry::cosTheta(v)) <= 0)
		return 0.f;

	real tanThetaV = DifferentialGeometry::sinTheta(v) /
		DifferentialGeometry::cosTheta(v);
	real b = std::sqrt(0.5f * alpha + 1) / tanThetaV;

	if (b < 1.6f)
	{
		return (3.535f * b + 2.181f * b * b) / (1 + 2.276f * b + 2.577f * b * b);
	}
	else
	{
		return 1.f;
	}
}