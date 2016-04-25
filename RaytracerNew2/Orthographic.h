#pragma once
#include "Camera.h"
class Orthographic :
	public Camera
{
public:
	Orthographic(const Parameters& params);
	~Orthographic();

	Ray getRay(double px, double py, const Point2d& apertureSample) override;

protected:
	double myFocal;
	double myMPerPixel;
};

