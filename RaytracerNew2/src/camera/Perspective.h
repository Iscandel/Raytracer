#pragma once
#include "Camera.h"

#include "core/Parameters.h"

class Perspective :
	public Camera
{
public:
	Perspective(const Parameters& params);
	~Perspective();

	Ray getRay(real px, real py, const Point2d& apertureSample);

protected:
	real myFocal;
	real myMPerPixel;
};

