#include "Homogeneous.h"

#include "ObjectFactoryManager.h"

Homogeneous::Homogeneous(const Parameters& params)
:Medium(params)
{
	mySigmaA = params.getColor("sigmaA", Color(0.15));//0.002, 0.002, 0.002));
	mySigmaS = params.getColor("sigmaS", Color(0.15));//0.0005, 0.0005, 0.0005));
	myScale = params.getDouble("scale", 1.);

	if (myScale != 1.)
	{
		mySigmaA *= myScale;
		mySigmaS *= myScale;
	}

	mySigmaT = mySigmaA + mySigmaS;
}


Homogeneous::~Homogeneous()
{
}

Color Homogeneous::transmittance(const Ray& ray, Sampler::ptr)
{
	double t = ray.myMaxT - ray.myMinT;
	return Color(std::exp(-mySigmaT.r * t), std::exp(-mySigmaT.g * t), std::exp(-mySigmaT.b * t));
}

bool Homogeneous::sampleDistance(const Ray &ray, Sampler::ptr sampler, double &t, Color &weight)
{
	//check inter ?
	int channel = (int)(sampler->getNextSample1D() * 3.);
	double extinctionValue;
	if (channel == 0) extinctionValue = mySigmaT.r; 
	if (channel == 1) extinctionValue = mySigmaT.g; 
	if (channel == 2) extinctionValue = mySigmaT.b;

	extinctionValue += tools::EPSILON;

	//double max = tools::EPSILON + std::max(mySigmaT.b, std::max(mySigmaT.r, mySigmaT.g));
	t = -std::log(1. - sampler->getNextSample1D()) / extinctionValue;

	if (t > ray.myMaxT)
	{
		t = ray.myMaxT;
		double pdf = (1 / 3.) * (std::exp(-mySigmaT.r * t) + std::exp(-mySigmaT.g * t) +std::exp(-mySigmaT.b * t));
		weight = transmittance(ray, sampler) / pdf;
		//std::cout << weight << " " << pdf << std::endl;
		return false;
	}
	else
	{
		double pdf = (1. / 3.) * (mySigmaT.r * std::exp(-mySigmaT.r * t) + mySigmaT.g * std::exp(-mySigmaT.g * t) + mySigmaT.b * std::exp(-mySigmaT.b * t));
		Ray tmpRay(ray);
		t += ray.myMinT;
		tmpRay.myMaxT = t;
		weight = mySigmaS * transmittance(tmpRay, sampler) / pdf;
		//std::cout << weight << " " << transmittance(tmpRay) << " " << tmpRay.myMaxT << " " << tmpRay.myMinT << " " << mySigmaS << " " << pdf << " " << mySigmaS / pdf << std::endl;

		return true;
	}
}

RT_REGISTER_TYPE(Homogeneous, Medium)