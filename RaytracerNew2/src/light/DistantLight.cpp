#include "DistantLight.h"

#include "core/Mapping.h"
#include "core/Math.h"
#include "factory/ObjectFactoryManager.h"
#include "core/Parameters.h"
#include "core/Scene.h"

DistantLight::DistantLight(const Parameters& params)
{
	myRadiance = params.getColor("radiance", Color());
	myLightToWorld = params.getTransform("toWorld", Transform::ptr(new Transform));
	myAngle = params.getReal("theta", 180.);
}

DistantLight::~DistantLight()
{
}

Color DistantLight::le(const Vector3d& direction, const Point3d&, const Normal3d&) const
{
	Vector3d localDir = myLightToWorld->inv().transform(direction);
	localDir.normalize();
	if (std::acos(localDir.z()) <= math::toRadian(myAngle))
		return myRadiance;

	return Color();
}

void DistantLight::initialize(Scene & scene)
{
	BoundingBox box = scene.getBoundingBox();
	myCenter = box.getCentroid();
	//we double the bbox radius to be sure
	mySphereRadius = 2 * std::max((box.getMin() - myCenter).norm(), (box.getMax() - myCenter).norm());
}

LightSamplingInfos DistantLight::sample(const Point3d& pFrom, const Point2d& sample)
{
	LightSamplingInfos infos;
	Vector3d dir = Mapping::squareToSphericalCap(sample, std::cos(math::toRadian(myAngle)));
	infos.intensity = myRadiance;
	infos.interToLight = myLightToWorld->transform(dir);
	infos.distance = mySphereRadius; //Compute the true distance ?
	//Should be already normalized (?)
	infos.interToLight.normalize();
	infos.sampledPoint = Ray(pFrom, infos.interToLight).getPointAt(mySphereRadius);// Point3d(100000, 1000000, 100000);//infos.wi; //No !!!!!!!!!!!!!!!!!!!!!!!!!!!
	infos.pdf = pdf(pFrom, infos);

	return infos;
}

real DistantLight::pdf(const Point3d&, const LightSamplingInfos& infos)
{
	real cosTheta = std::cos(math::toRadian(myAngle));
	Vector3d localWi = myLightToWorld->inv().transform(infos.interToLight);
	//if(cosTheta <= localWi.z())
	if(std::acos(localWi.z()) <= math::toRadian(myAngle))
		return Mapping::uniformConePdf(cosTheta);
	return 0.;
}

Color DistantLight::power() const
{
	return myRadiance * math::PI * mySphereRadius * mySphereRadius;
}

RT_REGISTER_TYPE(DistantLight, Light)