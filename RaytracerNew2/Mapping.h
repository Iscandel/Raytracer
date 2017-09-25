#pragma once

#include "Geometry.h"
#include "Sampler.h"
#include "Tools.h"

class Mapping
{
public:
	Mapping();
	~Mapping();

	///////////////////////////////////////////////////////////////////////////
	///
	///////////////////////////////////////////////////////////////////////////
	static Vector3d uniformSampleHemisphere(Sampler::ptr sampler, const Normal3d& normal)
	{
		Vector3d res;
		//Rejection sampling
		do
		{
			res.x() = 1 - 2 * sampler->getNextSample1D();
			res.y() = 1 - 2 * sampler->getNextSample1D();
			res.z() = 1 - 2 * sampler->getNextSample1D();
		} while (res.squaredNorm() > 1);

		if (res.dot(normal) < 0.)
			res = -res;
		res.normalize();

		return res;
	}

	///////////////////////////////////////////////////////////////////////////
	///
	///////////////////////////////////////////////////////////////////////////
	static Vector3d squareToUniformHemisphere(const Point2d& sample)
	{
		Vector3d res;

		double tmp = std::sqrt(1 - sample.y() * sample.y());

		res.x() = std::cos(2 * tools::PI * sample.x()) * tmp;
		res.y() = std::sin(2 * tools::PI * sample.x()) * tmp;
		res.z() = sample.y();
		res.normalize();

		return res;
	}

	///////////////////////////////////////////////////////////////////////////
	///
	///////////////////////////////////////////////////////////////////////////
	static double squareToUniformHemispherePdf()
	{
		return 1. / (2 * tools::PI);
	}

	///////////////////////////////////////////////////////////////////////////
	///
	///////////////////////////////////////////////////////////////////////////
	static Vector3d squareToCosineWeightedHemisphere(const Point2d& sample)
	{
		Vector3d res;

		double tmp = std::sqrt(1 - sample.y());

		res.x() = std::cos(2 * tools::PI * sample.x()) * tmp;
		res.y() = std::sin(2 * tools::PI * sample.x()) * tmp;
		res.z() = std::sqrt(sample.y());
		res.normalize();

		return res;
	}

	///////////////////////////////////////////////////////////////////////////
	///
	///////////////////////////////////////////////////////////////////////////
	static double squareToCosineWeightedHemisphere(double cosTheta)
	{
		return cosTheta * tools::INV_PI;
	}


	///////////////////////////////////////////////////////////////////////////
	///
	///////////////////////////////////////////////////////////////////////////
	static Point3d squareToTriangle(const Point2d& sample)
	{
		double r1 = sample.x();
		double r2 = sample.y();
		
		double sqrtR1 = std::sqrt(r1);
		double alpha = 1 - sqrtR1;
		double beta = (1 - r2) * sqrtR1;
		double gamma = r2 * sqrtR1;

		return Point3d(alpha, beta, gamma);
	}

	///////////////////////////////////////////////////////////////////////////
	///
	///////////////////////////////////////////////////////////////////////////
	static Point2d squareToUniformDisk(const Point2d& sample)
	{
		const double r1 = sample.x();
		const double r2 = sample.y();

		Point2d res;

		//First region
		if (r1 > -r2 && r1 > r2)
		{
			res.x() = r1;
			res.y() = (tools::PI * r2) / (4 * r1);
		}
		else if (r1 < r2 && r1 > -r2)
		{
			res.x() = r2;
			res.y() = (tools::PI / 4.) * (2 - r1 / r2);
		}
		else if (r1 < -r2 && r1 < r2)
		{
			res.x() = -r1;
			res.y() = (tools::PI / 4.) * (4 + r2 / r1);
		}
		else //if (r1 > r2 && r1 < -r2)
		{
			res.x() = -r2;
			res.y() = (tools::PI / 4.) * (6 - r1 / r2);
		}

		return res;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Algorithm from http://mathworld.wolfram.com/SpherePointPicking.html
	///////////////////////////////////////////////////////////////////////////
	static Vector3d squareToUniformSphere(const Point2d& sample)
	{
		Vector3d res;

		//Physical conventions : phi € [0, 2pi[
		double phi = 2. * tools::PI * sample.x();
		//double theta = std::acos(2 * sample.y() - 1.);

		//double u = std::cos(theta);
		//u = cos(arcos(2 * p2 * - 1))
		double u = 2 * sample.y() - 1.;
		double sinTheta = std::sqrt(1. - u * u);
		res.x() = sinTheta * std::cos(phi);
		res.y() = sinTheta * std::sin(phi);
		res.z() = u;

		res.normalize();

		return res;
	}

	///////////////////////////////////////////////////////////////////////////
	/// 
	///////////////////////////////////////////////////////////////////////////
	static double squareToUniformSpherePdf()
	{
		return 1. / (4 * tools::PI);
	}

	///////////////////////////////////////////////////////////////////////////
	/// //Similar to sampling sphere, except for z value
	///////////////////////////////////////////////////////////////////////////
	static Vector3d squareToSphericalCap(const Point2d& sample, double cosThetaMax)
	{
		Vector3d res;

		//Physical conventions : phi € [0, 2pi[
		double phi = 2. * tools::PI * sample.x();
		//Linear interpolation between cosThetaMax and 1
		double cosTheta = (1 - sample.y()) + sample.y() * cosThetaMax;//cosThetaMax + sample.y() * (1 - cosThetaMax);

		double sinTheta = std::sqrt(1. - cosTheta * cosTheta);
		res.x() = sinTheta * std::cos(phi);
		res.y() = sinTheta * std::sin(phi);
		res.z() = cosTheta;

		res.normalize();

		return res;
	}

	///////////////////////////////////////////////////////////////////////////
	///
	///////////////////////////////////////////////////////////////////////////
	static double uniformConePdf(double cosTheta)
	{
		return 1. / (2 * tools::PI * (1. - cosTheta));
	}
};

