#pragma once

#include "core/Geometry.h"
#include "sampler/Sampler.h"
#include "core/Math.h"

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

		real tmp = math::fastSqrt(1 - sample.y() * sample.y());

		res.x() = std::cos(2 * math::PI * sample.x()) * tmp;
		res.y() = std::sin(2 * math::PI * sample.x()) * tmp;
		res.z() = sample.y();
		res.normalize();

		return res;
	}

	///////////////////////////////////////////////////////////////////////////
	///
	///////////////////////////////////////////////////////////////////////////
	static real squareToUniformHemispherePdf()
	{
		return 1.f / (2 * math::PI);
	}

	///////////////////////////////////////////////////////////////////////////
	///
	///////////////////////////////////////////////////////////////////////////
	static Vector3d squareToCosineWeightedHemisphere(const Point2d& sample)
	{
		Point2d tmp = squareToConcentricDisk(sample);
		real z = std::sqrt(std::max((real)0., 1.f - tmp.x() * tmp.x() - tmp.y() * tmp.y()));

		return Vector3d(tmp.x(), tmp.y(), z);

		//The following implementation does not take into account degenerated cases (sample x/y = 0)
		//Vector3d res;

		//real tmp = std::sqrt(1 - sample.y());

		//res.x() = std::cos(2 * tools::PI * sample.x()) * tmp;
		//res.y() = std::sin(2 * tools::PI * sample.x()) * tmp;
		//res.z() = std::sqrt(sample.y());
		//res.normalize();

		//return res;
	}

	///////////////////////////////////////////////////////////////////////////
	///
	///////////////////////////////////////////////////////////////////////////
	static real squareToCosineWeightedHemisphere(real cosTheta)
	{
		return cosTheta * math::INV_PI;
	}


	///////////////////////////////////////////////////////////////////////////
	///
	///////////////////////////////////////////////////////////////////////////
	static Point3d squareToTriangle(const Point2d& sample)
	{
		real r1 = sample.x();
		real r2 = sample.y();
		
		real sqrtR1 = std::sqrt(r1);
		real alpha = 1 - sqrtR1;
		real beta = (1 - r2) * sqrtR1;
		real gamma = r2 * sqrtR1;

		return Point3d(alpha, beta, gamma);
	}

	///////////////////////////////////////////////////////////////////////////
	///
	///////////////////////////////////////////////////////////////////////////
	static Point2d squareToConcentricDisk(const Point2d& sample)
	{
		const real r1 = 2 * sample.x() - 1.f;
		const real r2 = 2 * sample.y() - 1.f;

		Point2d res;

		//First region
		if (r1 > -r2 && r1 > r2)
		{
			res.x() = r1;
			res.y() = (math::PI * r2) / (4 * r1);
		}
		else if (r1 < r2 && r1 > -r2)
		{
			res.x() = r2;
			res.y() = (math::PI / 4.f) * (2 - r1 / r2);
		}
		else if (r1 < -r2 && r1 < r2)
		{
			res.x() = -r1;
			res.y() = (math::PI / 4.f) * (4 + r2 / r1);
		}
		else //if (r1 > r2 && r1 < -r2)
		{
			res.x() = -r2;
			res.y() = (math::PI / 4.f) * (6 - r1 / r2);
		}

		return Point2d(res.x() * std::cos(res.y()), res.x() * std::sin(res.y()));
	}

	///////////////////////////////////////////////////////////////////////////
	/// Algorithm from http://mathworld.wolfram.com/SpherePointPicking.html
	///////////////////////////////////////////////////////////////////////////
	static Vector3d squareToUniformSphere(const Point2d& sample)
	{
		Vector3d res;

		//Physical conventions : phi € [0, 2pi[
		real phi = 2.f * math::PI * sample.x();
		//real theta = std::acos(2 * sample.y() - 1.);

		//real u = std::cos(theta);
		//u = cos(arcos(2 * p2 * - 1))
		real u = 2 * sample.y() - 1.f;
		real sinTheta = std::sqrt(1.f - u * u);
		res.x() = sinTheta * std::cos(phi);
		res.y() = sinTheta * std::sin(phi);
		res.z() = u;

		res.normalize();

		return res;
	}

	///////////////////////////////////////////////////////////////////////////
	/// 
	///////////////////////////////////////////////////////////////////////////
	static real squareToUniformSpherePdf()
	{
		return 1.f / (4 * math::PI);
	}

	///////////////////////////////////////////////////////////////////////////
	/// //Similar to sampling sphere, except for z value
	///////////////////////////////////////////////////////////////////////////
	static Vector3d squareToSphericalCap(const Point2d& sample, real cosThetaMax)
	{
		Vector3d res;

		//Physical conventions : phi € [0, 2pi[
		real phi = 2.f * math::PI * sample.x();
		//Linear interpolation between cosThetaMax and 1
		real cosTheta = (1 - sample.y()) + sample.y() * cosThetaMax;//cosThetaMax + sample.y() * (1 - cosThetaMax);

		real sinTheta = std::sqrt(1.f - cosTheta * cosTheta);
		res.x() = sinTheta * std::cos(phi);
		res.y() = sinTheta * std::sin(phi);
		res.z() = cosTheta;

		res.normalize();

		return res;
	}

	///////////////////////////////////////////////////////////////////////////
	///
	///////////////////////////////////////////////////////////////////////////
	static real uniformConePdf(real cosTheta)
	{
		return 1.f / (2 * math::PI * (1.f - cosTheta));
	}
};

