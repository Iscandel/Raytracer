#include "Phong.h"

#include "DifferentialGeometry.h"
#include "ObjectFactoryManager.h"
#include "Tools.h"

Phong::Phong(const Parameters& list)
{
	myKd = list.getDouble("kd", 0.5);
	myKs = list.getDouble("ks", 1 - myKd);
	myAlpha = list.getDouble("alpha", 2);
	myAlbedo = list.getColor("albedo", Color(100, 100, 100));

	myR = (myAlpha + 2) / (2 * tools::PI);
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

	return tools::INV_PI * myKd * myAlbedo + myR * myKs * myAlbedo * std::pow(infos.wo.dot(reflected), myAlpha) * DifferentialGeometry::cosTheta(infos.wi);
}

Color Phong::sample(BSDFSamplingInfos& infos, const Point2d& sample)
{
	throw std::runtime_error("Phong not implemented");
}

double Phong::pdf(const BSDFSamplingInfos& infos)
{
	throw std::runtime_error("Phong not implemented");
}

RT_REGISTER_TYPE(Phong, BSDF)