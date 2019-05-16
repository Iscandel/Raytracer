#include "Homogeneous.h"

#include "Math.h"
#include "ObjectFactoryManager.h"

Homogeneous::Homogeneous(const Parameters& params)
:Medium(params)
{
	mySigmaA = params.getColor("sigmaA", Color(0.15f));//0.002, 0.002, 0.002));
	mySigmaS = params.getColor("sigmaS", Color(0.15f));//0.0005, 0.0005, 0.0005));
	myScale = params.getReal("scale", 1.f);
	real temperature = params.getReal("temperature", (real)0);
	myEmission = temperature > (real)0 ? Color::fromBlackbody(temperature) : Color();
	myScaleEmissivity = params.getReal("scaleEmissivity", 1.);

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
	real t = ray.myMaxT - ray.myMinT;
	//return Color(std::exp(-mySigmaT.r * t), std::exp(-mySigmaT.g * t), std::exp(-mySigmaT.b * t));
	return Color(math::fastExp(-mySigmaT * t));
}

bool Homogeneous::sampleDistance(const Ray &ray, Sampler::ptr sampler, real &t, Color &weight, Color& emissivity)
{
	//check inter ?
	//int channel = (int)(sampler->getNextSample1D() * 3.f);
	//real extinctionValue;
	//if (channel == 0) extinctionValue = mySigmaT.r(); 
	//if (channel == 1) extinctionValue = mySigmaT.g(); 
	//if (channel == 2) extinctionValue = mySigmaT.b();
	int channel = (int)(sampler->getNextSample1D() * Color::NB_SAMPLES);
	assert(channel < Color::NB_SAMPLES);
	real extinctionValue = mySigmaT(channel);

	extinctionValue += math::EPSILON;

	//real max = math::EPSILON + std::max(mySigmaT.b, std::max(mySigmaT.r, mySigmaT.g));
	t = -std::log(1.f - sampler->getNextSample1D()) / extinctionValue;

	if (t > ray.myMaxT)
	{
		t = ray.myMaxT;
		real pdf = math::fastExp(-mySigmaT * t).average();
		//real pdf = (1 / 3.f) * (std::exp(-mySigmaT.r() * t) + std::exp(-mySigmaT.g() * t) +std::exp(-mySigmaT.b() * t));
		weight = transmittance(ray, sampler) / pdf;
		emissivity = Color();
		//std::cout << weight << " " << pdf << std::endl;
		return false;
	}
	else
	{
		//Color tmp = mySigmaT * math::fastExp(-mySigmaT * t);
		real pdf = Color(mySigmaT * math::fastExp(-mySigmaT * t)).average();
		//real pdf = (1.f / 3.f) * (mySigmaT.r() * std::exp(-mySigmaT.r() * t) + mySigmaT.g() * std::exp(-mySigmaT.g() * t) + mySigmaT.b() * std::exp(-mySigmaT.b() * t));
		Ray tmpRay(ray);
		t += ray.myMinT;
		tmpRay.myMaxT = t;
		weight = mySigmaS * transmittance(tmpRay, sampler) / pdf;
		emissivity = le(ray.direction()) / mySigmaT;
		//std::cout << weight << " " << transmittance(tmpRay) << " " << tmpRay.myMaxT << " " << tmpRay.myMinT << " " << mySigmaS << " " << pdf << " " << mySigmaS / pdf << std::endl;

		return true;
	}
}

LightSamplingInfos Homogeneous::sample(const Point3d& pFrom, const Point2d& sample)
{	
	LightSamplingInfos infos;
	Point3d size = myWorldBoundingBox.getMax() - myWorldBoundingBox.getMin();
	//res in [0, 1]^3
	real u = sample.x() * size.x() * size.z();
	real v = sample.y() * size.y();

	Point3d res;
	res.x() = ((u - (int)u) + (int)u % (int)size.x()) / size.x() + myWorldBoundingBox.getMin().x();
	res.y() = v / size.y() + myWorldBoundingBox.getMin().y();
	res.z() = ((int)u / size.x()) / (real)size.z() + myWorldBoundingBox.getMin().z();

	//std::shared_ptr<ISampledShape> shape = myShape.lock();
	//shape->sample(sample, infos.sampledPoint, infos.normal);

	infos.sampledPoint = res;
	infos.interToLight = infos.sampledPoint - pFrom;
	infos.distance = infos.interToLight.norm();
	infos.interToLight /= infos.distance;
	//real cosine = infos.normal.dot(-infos.interToLight);

	infos.intensity = le(infos.interToLight);
	infos.pdf = pdf(pFrom, infos);

	return infos;
}

RT_REGISTER_TYPE(Homogeneous, Medium)