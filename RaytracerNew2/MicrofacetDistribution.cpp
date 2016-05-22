#include "MicrofacetDistribution.h"

#include "DifferentialGeometry.h"
#include "Tools.h"


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

Normal3d MicrofacetDistribution::sampleNormal(const Point2d& sample, double alpha)
{
	if (myType == BECKMANN)
	{
		double theta = std::atan(std::sqrt(-alpha * alpha * std::log(1 - sample.x())));
		double phi = 2 * tools::PI * sample.y();

		return Normal3d(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta));
	}
	else if (myType == GGX)
	{
		double theta = std::atan(alpha * std::sqrt(sample.x()) / std::sqrt(1. - sample.x()));
		double phi = 2 * tools::PI * sample.y();

		return Normal3d(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta));
	}
	else
	{
		double theta = std::acos(std::pow(sample.x(), 1. / (alpha + 2)));
		double phi = 2 * tools::PI * sample.y();

		return Normal3d(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta));
	}
}

double MicrofacetDistribution::D(const Vector3d & wh, double alpha)
{
	if (myType == BECKMANN)
	{
		if (DifferentialGeometry::cosTheta(wh) <= 0.)
			return 0.;

		double cosThetaH2 = DifferentialGeometry::cosTheta(wh) * DifferentialGeometry::cosTheta(wh);
		double tanThetaH2 = DifferentialGeometry::sinTheta2(wh) / cosThetaH2;
		double alpha2 = alpha * alpha;
		return (std::exp(-tanThetaH2 / alpha2)) / (tools::PI * alpha2 * cosThetaH2 * cosThetaH2);
	}
	else if (myType == GGX)
	{
		if (DifferentialGeometry::cosTheta(wh) <= 0.)
			return 0.;

		double alpha2 = alpha * alpha;
		double cosThetaH2 = DifferentialGeometry::cosTheta2(wh);
		double tanThetaH2 = DifferentialGeometry::tanTheta2(wh);
		double smallSum = (alpha2 + tanThetaH2);

		return alpha2 / (tools::PI * cosThetaH2 * cosThetaH2 * smallSum * smallSum);
	}
	else
	{
		if (DifferentialGeometry::cosTheta(wh) <= 0.)
			return 0.;

		double cosThetaH = DifferentialGeometry::cosTheta(wh);

		return (alpha + 2.) / (2 * tools::PI) * std::pow(cosThetaH, alpha);
	}
}

double MicrofacetDistribution::G(const Vector3d & wi, const Vector3d & wo, const Vector3d & wh, double alpha)
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

double MicrofacetDistribution::shadowingTermG1Beckmann(const Vector3d& v, const Vector3d& m, double alpha)
{
	if ((v.dot(m)) / (DifferentialGeometry::cosTheta(v)) <= 0)
		return 0.;

	double tanThetaV = DifferentialGeometry::sinTheta(v) /
		DifferentialGeometry::cosTheta(v);
	double b = 1. / (alpha * tanThetaV);

	if (b < 1.6)
	{
		return (3.535 * b + 2.181 * b * b) / (1 + 2.276 * b + 2.577 * b * b);
	}
	else
	{
		return 1.;
	}
}

double MicrofacetDistribution::shadowingTermG1GGX(const Vector3d& v, const Vector3d& m, double alpha)
{
	if ((v.dot(m)) / (DifferentialGeometry::cosTheta(v)) <= 0)
		return 0.;

	double alpha2 = alpha * alpha;
	double tan2 = DifferentialGeometry::tanTheta2(v);
	return 2. / (1 + std::sqrt(1 + alpha2 * tan2));
}

double MicrofacetDistribution::shadowingTermG1Phong(const Vector3d& v, const Vector3d& m, double alpha)
{
	if ((v.dot(m)) / (DifferentialGeometry::cosTheta(v)) <= 0)
		return 0.;

	double tanThetaV = DifferentialGeometry::sinTheta(v) /
		DifferentialGeometry::cosTheta(v);
	double b = std::sqrt(0.5 * alpha + 1) / tanThetaV;

	if (b < 1.6)
	{
		return (3.535 * b + 2.181 * b * b) / (1 + 2.276 * b + 2.577 * b * b);
	}
	else
	{
		return 1.;
	}
}