#include "Camera.h"
#include "Geometry.h"
#include "Tools.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Camera::Camera(const Parameters& params)
{
	int sizeX = params.getInt("sizeX", 768);
	int sizeY = params.getInt("sizeY", 768);
	myLensRadius = params.getDouble("apertureRadius", 0.);
	myFocalPlane = params.getDouble("focalPlane", tools::MAX_DOUBLE); //focusPlane

	setSize(sizeX, sizeY);

	myWorldTransform = Transform::ptr(new Transform);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Camera::~Camera(void)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
//Ray Camera::getRay(double px, double py)
//{
//	double aspect = this->getSizeX() / (double)this->getSizeY();
//	double m_farClip = 1e4;
//	double m_nearClip = 1e-4;
//	double m_fov = 30.;
//	double recip = 1.0 / (m_farClip - m_nearClip),
//	cot = 1.0 / std::tan(tools::toRadian(m_fov / 2.0));
//	
//	Eigen::Matrix4d perspective;
//	perspective <<
//	cot, 0, 0, 0,
//	0, cot, 0, 0,
//	0, 0, m_farClip * recip, -m_nearClip * m_farClip * recip,
//	0, 0, 1., 0;
//	
//	/**
//	* Translation and scaling to shift the clip coordinates into the
//	* range from zero to one. Also takes the aspect ratio into account.
//	*/
//	Transform m_sampleToCamera = Transform(
//		Eigen::DiagonalMatrix<double, 3>(Vector3d(0.5, -0.5 * aspect, 1.0)) *
//		Eigen::Translation<double, 3>(1.0, -1.0 / aspect, 0.0) * perspective).inv();
//	
//	Vector3d m_invOutputSize = Vector3d(1. / getSizeX(), 1. / getSizeY(), 0.);
//	
//	/* Compute the corresponding position on the
//	near plane (in local camera space) */
//	Point3d nearP = m_sampleToCamera.transform(Point3d(
//		px * m_invOutputSize.x(),
//		py * m_invOutputSize.y(), 0.0));
//	
//	/* Turn into a normalized ray direction, and
//	adjust the ray interval accordingly */
//	Vector3d d = nearP.normalized();
//	double invZ = 1.0 / d.z();
//	
//	
//	Ray ray2;
//	ray2.myOrigin = myWorldTransform->transform(Point3d(0, 0, 0));
//	ray2.direction(myWorldTransform->transform(d));
//	
//	return ray2;
//
//	Point3d imagePlane;
//	Point3d origin;
//
//	Vector3d direction;
//
//	Ray ray;
//
//	if(myCameraType == CameraType::PERSPECTIVE)
//	{
//		imagePlane = Point3d(
//			px * myMPerPixel - myMPerPixel * getSizeX() / 2., 
//			myFocal,
//			myMPerPixel * getSizeY() / 2. - py * myMPerPixel
//		);
//
//		direction = Vector3d(origin, imagePlane);
//		direction.normalize();
//		double invDir = 1. / direction.y();
//
//		ray.myOrigin = origin;
//		ray.direction(direction);
//		ray.myMinT = myFocal * invDir;
//		ray.myMaxT *= invDir;
//	}
//	else if(myCameraType == CameraType::ORTHOGRAPHIC)
//	{
//		imagePlane = Point3d(
//			px * myMPerPixel - myMPerPixel * getSizeX() / 2., 
//			myFocal,
//			myMPerPixel * getSizeY() / 2. - py * myMPerPixel
//		);
//
//		origin = Point3d(
//			px * myMPerPixel - myMPerPixel * getSizeX() / 2.,
//			0.,
//			myMPerPixel * getSizeY() / 2. - py * myMPerPixel);
//
//		direction = Vector3d(origin, imagePlane);
//		direction.normalize();
//
//		double invDir = 1. / direction.z();
//
//		ray.myOrigin = origin;
//		ray.direction(direction);
//		ray.myMinT = myFocal * invDir;
//		ray.myMaxT *= invDir;
//
//		//direction = Vector3d(
//		//	0., 
//		//	1,
//		//	0.
//		//);
//
//		//origin = Vector3d(
//		//	px * myMPerPixel - myMPerPixel * getSizeX() / 2.,
//		//	myFocal, 
//		//	myMPerPixel * getSizeY() / 2. - py * myMPerPixel
//		//);
//
//	}
//	else
//	{
//		throw std::runtime_error("Camera type not recognized.");
//	}
//
//	ray = myWorldTransform->transform(ray);
//
//	return ray;
//}



//		Point3d sample(tools::random(0., 1.), tools::random(0., 1.), 0.);
//		double lensRadius = 0.05;
//		if (lensRadius > 0.) {
//			double focalDistance = 4;
//			// Sample point on lens
//			//float lensU, lensV;
//			Point3d lens = squareToDiskConcentric(sample);
//			lens = lens * lensRadius;
//			//lensV *= lensRadius;
//			imagePlane.normalize();
//			// Compute point on plane of focus
//			double ft = focalDistance / imagePlane.y;
//			Vector3d Pfocus = Vector3d() + ft * imagePlane;
//
//			// Update ray for effect of lens
//			origin += Vector3d(lens.x, 0. , lens.y);
//			origin.x = -origin.x;
//			origin.y = -origin.y;
//			origin.z = -origin.z;
//			imagePlane = Pfocus + origin;
//		}
//
//
//Point3d squareToDiskConcentric(const Point3d &sample) {
//	double r1 = 2.0f*sample.x - 1.0f;
//	double r2 = 2.0f*sample.y - 1.0f;
//	const double M_PI = 3.14159;
//
//	Point3d coords;
//	if (r1 == 0 && r2 == 0) {
//		coords = Point3d(0, 0, 0);
//	} else if (r1 > -r2) { /* Regions 1/2 */
//		if (r1 > r2)
//			coords = Point3d(r1, (M_PI/4.0f) * r2/r1);
//		else
//			coords = Point3d(r2, (M_PI/4.0f) * (2.0f - r1/r2));
//	} else { /* Regions 3/4 */
//		if (r1<r2)
//			coords = Point3d(-r1, (M_PI/4.0f) * (4.0f + r2/r1));
//		else 
//			coords = Point3d(-r2, (M_PI/4.0f) * (6.0f - r1/r2));
//	}
//	return Point3d(
//		coords.x*std::cos(coords.y),
//		coords.x*std::sin(coords.y)
//	);
//}


//void ConcentricSampleDisk(float u1, float u2, float *dx, float *dy) {
//    float r, theta;
//    // Map uniform random numbers to $[-1,1]^2$
//    float sx = 2 * u1 - 1;
//    float sy = 2 * u2 - 1;
//
//    // Map square to $(r,\theta)$
//
//    // Handle degeneracy at the origin
//    if (sx == 0.0 && sy == 0.0) {
//        *dx = 0.0;
//        *dy = 0.0;
//        return;
//    }
//    if (sx >= -sy) {
//        if (sx > sy) {
//            // Handle first region of disk
//            r = sx;
//            if (sy > 0.0) theta = sy/r;
//            else          theta = 8.0f + sy/r;
//        }
//        else {
//            // Handle second region of disk
//            r = sy;
//            theta = 2.0f - sx/r;
//        }
//    }
//    else {
//        if (sx <= sy) {
//            // Handle third region of disk
//            r = -sx;
//            theta = 4.0f - sy/r;
//        }
//        else {
//            // Handle fourth region of disk
//            r = -sy;
//            theta = 6.0f + sx/r;
//        }
//    }
//    theta *= M_PI / 4.f;
//    *dx = r * cosf(theta);
//    *dy = r * sinf(theta);
//}



//double aspect = this->getSizeX() / (double)this->getSizeY();
//double m_farClip = 1e4;
//double m_nearClip = 1e-4;
//double m_fov = 30.;
//double recip = 1.0 / (m_farClip - m_nearClip),
//cot = 1.0 / std::tan(tools::toRadian(m_fov / 2.0));
//
//Eigen::Matrix4d perspective;
//perspective <<
//cot, 0, 0, 0,
//0, cot, 0, 0,
//0, 0, m_farClip * recip, -m_nearClip * m_farClip * recip,
//0, 0, 1., 0;
//
///**
//* Translation and scaling to shift the clip coordinates into the
//* range from zero to one. Also takes the aspect ratio into account.
//*/
//Transform m_sampleToCamera = Transform(
//	Eigen::DiagonalMatrix<double, 3>(Vector3d(0.5, -0.5 * aspect, 1.0)) *
//	Eigen::Translation<double, 3>(1.0, -1.0 / aspect, 0.0) * perspective).inv();
//
//Vector3d m_invOutputSize = Vector3d(1. / getSizeX(), 1. / getSizeY(), 0.);
//
///* Compute the corresponding position on the
//near plane (in local camera space) */
//Point3d nearP = m_sampleToCamera.transform(Point3d(
//	px * m_invOutputSize.x(),
//	py * m_invOutputSize.y(), 0.0));
//
///* Turn into a normalized ray direction, and
//adjust the ray interval accordingly */
//Vector3d d = nearP.normalized();
//double invZ = 1.0 / d.z();
//
//
//Ray ray2;
//ray2.myOrigin = myWorldTransform.transform(Point3d(0, 0, 0));
//ray2.myDirection = myWorldTransform.transform(d);
//
//return ray2;