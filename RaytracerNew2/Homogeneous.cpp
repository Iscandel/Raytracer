#include "Homogeneous.h"



Homogeneous::Homogeneous(const Parameters& params)
:Medium(params)
{
	mySigmaA = params.getColor("sigmaA", Color(0.15));//0.002, 0.002, 0.002));
	mySigmaS = params.getColor("sigmaS", Color(0.15));//0.0005, 0.0005, 0.0005));
	mySigmaT = mySigmaA + mySigmaS;
}


Homogeneous::~Homogeneous()
{
}

Color Homogeneous::transmittance(const Ray& ray)
{
	double t = ray.myMaxT - ray.myMinT;
	return Color(std::exp(-mySigmaT.r * t), std::exp(-mySigmaT.g * t), std::exp(-mySigmaT.b * t));
}

bool Homogeneous::sampleDistance(const Ray &ray, Point2d& sample, double &t, Color &weight)
{
	//check inter

	double max = tools::EPSILON + std::max(mySigmaT.b, std::max(mySigmaT.r, mySigmaT.g));
	t = -std::log(1. - sample.x()) / max;

	if (t > ray.myMaxT)
	{
		t = ray.myMaxT;
		double pdf = (1 / 3.) * (std::exp(-mySigmaT.r * t) + std::exp(-mySigmaT.g * t) +std::exp(-mySigmaT.b * t));
		weight = transmittance(ray) / pdf;
		//std::cout << weight << " " << pdf << std::endl;
		return false;
	}
	else
	{
		double pdf = (1. / 3.) * (mySigmaT.r * std::exp(-mySigmaT.r * t) + mySigmaT.g * std::exp(-mySigmaT.g * t) + mySigmaT.b * std::exp(-mySigmaT.b * t));
		Ray tmpRay(ray);
		t += ray.myMinT;
		tmpRay.myMaxT = t;
		weight = mySigmaS * transmittance(tmpRay) / pdf;

		return true;
	}
}