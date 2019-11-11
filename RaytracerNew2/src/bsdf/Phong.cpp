#include "Phong.h"

#include "core/DifferentialGeometry.h"
#include "core/Math.h"
#include "core/Mapping.h"
#include "factory/ObjectFactoryManager.h"
#include "texture/ConstantTexture.h"

Phong::Phong(const Parameters& list)
:BSDF(list)
{
	if (list.hasTexture("kd"))
		myKd = list.getTexture("kd", nullptr);
	else
		myKd = Texture::ptr(new ConstantTexture(list.getColor("kd", Color(0.5))));

	if (list.hasTexture("ks"))
		myKs = list.getTexture("ks", nullptr);
	else
		myKs = Texture::ptr(new ConstantTexture(list.getColor("ks", Color(0.5))));

	//Phong exponent
	myN = list.getReal("specularExponent", 2); 

	myR = (myN + 2) / (2 * math::PI);

	//According to "Using the modified Phong reflection model for physically based rendering", 
	//the sampling strategy is as follows:
	//if s < kd, sample diffuse
	//if kd < s < ks*(n+2) / (n+1) * cos^n alpha, sample specular
	//else, no contribution
	//So, we scale a bit kd. kd + ks < 1 for reciprocity principle
	//So, if we want ks + kd = 1 -> (kd + ks) * scale = 1 -> scale = 1 / (kd + ks)
	mySamplingWeight = myKd->getAverage().average() / (myKd->getAverage().average() + myKs->getAverage().average());
}


Phong::~Phong()
{
}

Color Phong::eval(const BSDFSamplingInfos& infos)
{
	if (DifferentialGeometry::cosTheta(infos.wi) <= 0. ||
		DifferentialGeometry::cosTheta(infos.wo) <= 0.)
		return Color();

	Vector3d reflected = Vector3d(-infos.wi.x(), -infos.wi.y(), infos.wi.z());

	//According to the paper, clamp  alpha > pi / 2 to pi / 2 to avoid negative cosine
	real cosAlpha = infos.wo.dot(reflected);
	return math::INV_PI * myKd->eval(infos.uv) + 
		   (cosAlpha < 0 ? Color() : myR * myKs->eval(infos.uv) * std::pow(cosAlpha, myN));
}

Color Phong::sample(BSDFSamplingInfos& infos, const Point2d& _sample)
{
	Point2d sample(_sample);
	if (sample.x() < mySamplingWeight)
	{
		//Reuse the sample by rescaling it between 0 and 1
		sample.x() /= mySamplingWeight;
		infos.wo = Mapping::squareToCosineWeightedHemisphere(sample);
		infos.sampledType = BSDF::DIFFUSE;

		//Also Ok
		//real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
		//infos.pdf = cosThetaO / math::PI * mySamplingWeight;
		//if (infos.pdf == 0.)
		//	return Color();
		//return myKd * math::INV_PI / infos.pdf * cosThetaO;
	}
	else
	{
		//Reuse the sample by rescaling it between 0 and 1
		sample.x() = (sample.x() - mySamplingWeight) / (real(1.) - mySamplingWeight);

		real _sqrt = math::fastSqrt((real)1. - std::pow(sample.x(), real(2.) / (myN + 1)));
		real cosA = std::pow(sample.x(), real(1.) / (myN + 1));
		infos.wo = Vector3d(_sqrt * std::cos(2 * math::PI * sample.y()),
			_sqrt * std::sin(2 * math::PI * sample.y()),
			cosA);
		//In Lafortune's paper, alpha is the angle between the reflected direction and
		//the perfect specular direction.
		//Here, the specular direction is sampled in terms of (alpha, phi) and not (theta, phi),
		//which means a (0,0,1) means a perfect specular reflection
		//So, create a sub coordinate system in which the normal is the specular reflection direction.
		//Next, apply toWorld to get the sampled direction in the classical local frame
		Vector3d reflected = reflect(infos.wi);
		DifferentialGeometry phongFrame(reflected);
		infos.wo = phongFrame.toWorld(infos.wo);

		infos.sampledType = BSDF::GLOSSY_REFLECTION;

		//Also Ok
		//cosA = infos.wo.dot(reflected);
		//infos.pdf = (myN + 1) / (2 * math::PI) * std::pow(cosA, myN) * (1. - mySamplingWeight);
		//if (infos.pdf == 0.)
		//	return Color();
		//real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);

		////Vector3d reflected = Vector3d(-infos.wi.x(), -infos.wi.y(), infos.wi.z());
		//return myR * myKs * std::pow(cosA, myN) / infos.pdf * cosThetaO;
	}

	infos.pdf = pdf(infos);
	if (infos.pdf == 0.)
		return Color();
	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
	return eval(infos) / infos.pdf * cosThetaO;
}

real Phong::pdf(const BSDFSamplingInfos& infos)
{
	if (DifferentialGeometry::cosTheta(infos.wi) <= 0. ||
		DifferentialGeometry::cosTheta(infos.wo) <= 0.)
		return (real) 0.;

	real cosThetaO = DifferentialGeometry::cosTheta(infos.wo);
	Vector3d reflected = reflect(infos.wi);

	real cosAlpha = infos.wo.dot(reflected);

	return mySamplingWeight * cosThetaO * math::INV_PI + (cosAlpha < 0 ? 0. :
		(real(1.) - mySamplingWeight) * (myN + 1) / (2 * math::PI) * std::pow(cosAlpha, myN));
}

RT_REGISTER_TYPE(Phong, BSDF)