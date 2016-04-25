#pragma once
#include "Camera.h"
#include "Geometry.h"
#include "Parameters.h"

class NoriPerspective :
	public Camera
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

public:
	NoriPerspective(const Parameters& params);

	~NoriPerspective();

	Ray getRay(double px, double py, const Point2d& apertureSample) override;

protected:
	double myFarClip;
	double myNearClip;
	double myFov;
	Vector3d myInvOutputSize;
	Transform mySampleToCamera;
};

