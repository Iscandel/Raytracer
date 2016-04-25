#pragma once
#include "Camera.h"

#include "Parameters.h"

class Perspective :
	public Camera
{
public:
	Perspective(const Parameters& params);
	~Perspective();

	Ray getRay(double px, double py, const Point2d& apertureSample);

protected:
	double myFocal;
	double myMPerPixel;
};

