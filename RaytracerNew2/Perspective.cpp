#include "Perspective.h"

#include "Mapping.h"
#include "ObjectFactoryManager.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Perspective::Perspective(const Parameters & params)
:Camera(params)
{
	//focal (meters)
	myFocal = params.getReal("focal", 0.05f);
	//pixel size (meters)
	myMPerPixel = params.getReal("meterPixelSize", 0.0002f);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Perspective::~Perspective()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Ray Perspective::getRay(real px, real py, const Point2d& apertureSample)
{
	Point3d origin;
	Ray ray;

	Point3d imagePlane = Point3d(
		px * myMPerPixel - myMPerPixel * getSizeX() / 2.f,
		myFocal,
		myMPerPixel * getSizeY() / 2.f - py * myMPerPixel
		);

	Vector3d direction = imagePlane;//Vector3d(origin, imagePlane);
	direction.normalize();
	real invDir = 1.f / direction.y();

	ray.myOrigin = origin;
	ray.direction(direction);

	if (myLensRadius > 0.f)
	{
		Point2d sample = Mapping::squareToConcentricDisk(apertureSample);//squareToUniformDiskConcentric(apertureSample);
		sample *= myLensRadius;
		ray.myOrigin = Point3d(sample.x(), sample.y(), 0.);

		real tDist = myFocalPlane * invDir;
		Point3d focalPlanePoint = tDist *  direction;

		Vector3d newDir = (focalPlanePoint - ray.myOrigin).normalized();
		ray.direction(newDir);
	}

	ray.myMinT = myFocal * invDir;
	ray.myMaxT *= invDir;

	ray = myWorldTransform->transform(ray);

	return ray;
}

RT_REGISTER_TYPE(Perspective, Camera)