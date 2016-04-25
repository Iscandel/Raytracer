#include "Perspective.h"

#include "Mapping.h"
#include "ObjectFactoryManager.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Perspective::Perspective(const Parameters & params)
:Camera(params)
{
	//focal (meters)
	myFocal = params.getDouble("focal", 0.05);
	//pixel size (meters)
	myMPerPixel = params.getDouble("meterPixelSize", 0.0002);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Perspective::~Perspective()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Ray Perspective::getRay(double px, double py, const Point2d& apertureSample)
{
	Point3d origin;
	Ray ray;

	Point3d imagePlane = Point3d(
		px * myMPerPixel - myMPerPixel * getSizeX() / 2.,
		myFocal,
		myMPerPixel * getSizeY() / 2. - py * myMPerPixel
		);

	Vector3d direction = imagePlane;//Vector3d(origin, imagePlane);
	direction.normalize();
	double invDir = 1. / direction.y();

	ray.myOrigin = origin;
	ray.direction(direction);

	if (myLensRadius > 0.)
	{
		Point2d sample = Mapping::squareToUniformDisk(apertureSample);//squareToUniformDiskConcentric(apertureSample);
		sample *= myLensRadius;
		ray.myOrigin = Point3d(sample.x(), sample.y(), 0.);

		double tDist = myFocalPlane * invDir;
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