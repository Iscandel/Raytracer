#pragma once
#include "Camera.h"
class Orthographic :
	public Camera
{
public:
	Orthographic(const Parameters& params);
	~Orthographic();

	Ray getRay(real px, real py, const Point2d& apertureSample) override;

protected:
	real myFocal;
	real myMPerPixel;
};

