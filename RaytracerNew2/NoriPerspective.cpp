#include "NoriPerspective.h"

#include "Mapping.h"
#include "ObjectFactoryManager.h"
#include "Parameters.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
NoriPerspective::NoriPerspective(const Parameters& params)
:Camera(params)
{

	double aspect = getSizeX() / (double) getSizeY();
	myFarClip = params.getDouble("farClip", 1e4);
	myNearClip = params.getDouble("nearClip", 1e-4);
	myFov = params.getDouble("fov", 30.);
	double recip = 1.0 / (myFarClip - myNearClip),
		cot = 1.0 / std::tan(tools::toRadian(myFov / 2.0));

	Eigen::Matrix4d perspective;
	perspective <<
		cot, 0, 0, 0,
		0, cot, 0, 0,
		0, 0, myFarClip * recip, -myNearClip * myFarClip * recip,
		0, 0, 1., 0;

	/**
	* Translation and scaling to shift the clip coordinates into the
	* range from zero to one. Also takes the aspect ratio into account.
	*/
	mySampleToCamera = Transform(
		Eigen::DiagonalMatrix<double, 3>(Vector3d(0.5, -0.5 * aspect, 1.0)) *
		Eigen::Translation<double, 3>(1.0, -1.0 / aspect, 0.0) * perspective).inv();

	myInvOutputSize = Vector3d(1. / getSizeX(), 1. / getSizeY(), 0.);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
NoriPerspective::~NoriPerspective()
{
}

//inline void sincosd(double theta, double* _sin, double* _cos) {
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
Ray NoriPerspective::getRay(double px, double py, const Point2d& apertureSample)
{
	/* Compute the corresponding position on the
	near plane (in local camera space) */
	Point3d nearP = mySampleToCamera.transform(Point3d(
		px * myInvOutputSize.x(),
		py * myInvOutputSize.y(), 0.0));

	/* Turn into a normalized ray direction, and
	adjust the ray interval accordingly */
	Vector3d d = nearP.normalized();
	double invZ = 1.0 / d.z();

	Ray ray(Point3d(0., 0., 0.), d);

	if (myLensRadius > 0.)
	{
		Point2d sample = Mapping::squareToUniformDisk(apertureSample);//squareToUniformDiskConcentric(apertureSample);
		sample *= myLensRadius;
		ray.myOrigin = Point3d(sample.x(), sample.y(), 0.);

		double tDist = myFocalPlane * invZ;
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