#pragma once
#include "Camera.h"
#include "core/Geometry.h"
#include "core/Parameters.h"

class NoriPerspective :
	public Camera
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

public:
	NoriPerspective(const Parameters& params);

	~NoriPerspective();

	Ray getRay(real px, real py, const Point2d& apertureSample) override;

protected:
	real myFarClip;
	real myNearClip;
	real myFov;
	Vector3d myInvOutputSize;
	Transform mySampleToCamera;
};

