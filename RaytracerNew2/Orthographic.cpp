#include "Orthographic.h"

#include "ObjectFactoryManager.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Orthographic::Orthographic(const Parameters& params)
:Camera(params)
{
	myFocal = params.getDouble("focal", 0);
	myMPerPixel = params.getDouble("meterPixelSize", 0.001);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Orthographic::~Orthographic()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Ray Orthographic::getRay(double px, double py, const Point2d&)
{
	//imagePlane = Point3d(
	//	px * myMPerPixel - myMPerPixel * getSizeX() / 2.,
	//	myFocal,
	//	myMPerPixel * getSizeY() / 2. - py * myMPerPixel
	//);

	Point3d origin = Point3d(
		px * myMPerPixel - myMPerPixel * getSizeX() / 2.,
		0.,
		myMPerPixel * getSizeY() / 2. - py * myMPerPixel
	);	
	
	Vector3d direction = Vector3d(
		0., 
		1.,
		0.
	);

	Ray ray;
	ray.myOrigin = origin;
	ray.direction(direction);
	ray.myMinT = myFocal;

	ray = myWorldTransform->transform(ray);

	return ray;
}

RT_REGISTER_TYPE(Orthographic, Camera)