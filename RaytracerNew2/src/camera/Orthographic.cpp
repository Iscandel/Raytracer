#include "Orthographic.h"

#include "factory/ObjectFactoryManager.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Orthographic::Orthographic(const Parameters& params)
:Camera(params)
{
	myFocal = params.getReal("focal", 0);
	myMPerPixel = params.getReal("meterPixelSize", 0.001f);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Orthographic::~Orthographic()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Ray Orthographic::getRay(real px, real py, const Point2d&)
{
	//imagePlane = Point3d(
	//	px * myMPerPixel - myMPerPixel * getSizeX() / 2.,
	//	myFocal,
	//	myMPerPixel * getSizeY() / 2. - py * myMPerPixel
	//);

	Point3d origin = Point3d(
		px * myMPerPixel - myMPerPixel * getSizeX() / 2.f,
		0.f,
		myMPerPixel * getSizeY() / 2.f - py * myMPerPixel
	);	
	
	Vector3d direction = Vector3d(
		0, 
		1,
		0
	);

	Ray ray;
	ray.myOrigin = origin;
	ray.direction(direction);
	ray.myMinT = myFocal;

	ray = myWorldTransform->transform(ray);

	return ray;
}

RT_REGISTER_TYPE(Orthographic, Camera)