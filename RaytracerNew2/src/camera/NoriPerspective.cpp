#include "NoriPerspective.h"

#include "core/Mapping.h"
#include "core/Math.h"
#include "factory/ObjectFactoryManager.h"
#include "core/Parameters.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
NoriPerspective::NoriPerspective(const Parameters& params)
:Camera(params)
{

	real aspect = getSizeX() / (real) getSizeY();
	myFarClip = params.getReal("farClip", 1e4f);
	myNearClip = params.getReal("nearClip", 1e-4f);
	myFov = params.getReal("fov", 30.);
	real recip = 1.0f / (myFarClip - myNearClip),
		cot = 1.0f / std::tan(math::toRadian(myFov / 2.0f));

	Transform::Matrix4r perspective;
	perspective <<
		cot, 0, 0, 0,
		0, cot, 0, 0,
		0, 0, myFarClip * recip, -myNearClip * myFarClip * recip,
		0, 0, 1.f, 0;

	/**
	* Translation and scaling to shift the clip coordinates into the
	* range from zero to one. Also takes the aspect ratio into account.
	*/
	mySampleToCamera = Transform(
		Eigen::DiagonalMatrix<real, 3>(Eigen::Matrix<real, 3, 1>(-0.5f, -0.5f * aspect, 1.0f)) *
		Eigen::Translation<real, 3>(-1.0f, -1.0f / aspect, 0.0f) * perspective).inv();

	myInvOutputSize = Vector3d(1.f / getSizeX(), 1.f / getSizeY(), 0.);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
NoriPerspective::~NoriPerspective()
{
}

//inline void sincosd(real theta, real* _sin, real* _cos) {
//	*_sin = sinf(theta);
//	*_cos = cosf(theta);
//}
//
//
//Point2d squareToUniformDiskConcentric(const Point2d &sample) {
//	float r1 = 2.0f*sample.x() - 1.0f;
//	float r2 = 2.0f*sample.y() - 1.0f;
//
//	Point2d coords;
//	if (r1 == 0 && r2 == 0) {
//		coords = Point2d(0, 0);
//	}
//	else if (r1 > -r2) { /* Regions 1/2 */
//		if (r1 > r2)
//			coords = Point2d(r1, (M_PI / 4.0f) * r2 / r1);
//		else
//			coords = Point2d(r2, (M_PI / 4.0f) * (2.0f - r1 / r2));
//	}
//	else { /* Regions 3/4 */
//		if (r1<r2)
//			coords = Point2d(-r1, (M_PI / 4.0f) * (4.0f + r2 / r1));
//		else
//			coords = Point2d(-r2, (M_PI / 4.0f) * (6.0f - r1 / r2));
//	}
//
//	Point2d result;
//	sincosd(coords.y(), &result[1], &result[0]);
//	return result*coords.x();
//}


//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Ray NoriPerspective::getRay(real px, real py, const Point2d& apertureSample)
{
	/* Compute the corresponding position on the
	near plane (in local camera space) */
	Point3d nearP = mySampleToCamera.transform(Point3d(
		px * myInvOutputSize.x(),
		py * myInvOutputSize.y(), 0.0));

	/* Turn into a normalized ray direction, and
	adjust the ray interval accordingly */
	Vector3d d = nearP.normalized();
	real invZ = 1.0f / d.z();

	Ray ray(Point3d(0.f, 0.f, 0.f), d);

	if (myLensRadius > 0.)
	{
		Point2d sample = Mapping::squareToConcentricDisk(apertureSample);//squareToUniformDiskConcentric(apertureSample);
		sample *= myLensRadius;
		ray.myOrigin = Point3d(sample.x(), sample.y(), 0.f);

		real tDist = myFocalPlane * invZ;
		Point3d focalPlanePoint = tDist *  d;

		//Point3d focalPlanePoint = nearP * (myFocalPlane / nearP.z());

		Vector3d newDir = (focalPlanePoint - ray.myOrigin).normalized();
		ray.direction(newDir);
	}

	ray = myWorldTransform->transform(ray);
	
	//ray.myOrigin = myWorldTransform->transform(Point3d(0, 0, 0));
	//ray.direction(myWorldTransform->transform(d));
	ray.myMinT = myNearClip * invZ;
	ray.myMaxT = myFarClip * invZ;

	return ray;
}

RT_REGISTER_TYPE(NoriPerspective, Camera)