#include "Phong.h"

#include "core/DifferentialGeometry.h"
#include "factory/ObjectFactoryManager.h"
#include "core/Math.h"

Phong::Phong(const Parameters& list)
:BSDF(list)
{
	myKd = list.getReal("kd", 0.5);
	myKs = list.getReal("ks", 1 - myKd);
	myAlpha = list.getReal("alpha", 2);
	myAlbedo = list.getColor("albedo", Color(0.5f));

	myR = (myAlpha + 2) / (2 * math::PI);
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

	return math::INV_PI * myKd * myAlbedo + myR * myKs * myAlbedo * std::pow(infos.wo.dot(reflected), myAlpha) * DifferentialGeometry::cosTheta(infos.wi);
}

Color Phong::sample(BSDFSamplingInfos& infos, const Point2d& sample)
{
	throw std::runtime_error("Phong not implemented");
}

real Phong::pdf(const BSDFSamplingInfos& infos)
{
	throw std::runtime_error("Phong not implemented");
}

RT_REGISTER_TYPE(Phong, BSDF)