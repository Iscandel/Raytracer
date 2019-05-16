#include "Mandelbulb.h"

#include "GeometricShapeFactory.h"
#include "Intersection.h"
#include "ObjectFactoryManager.h"

#include <cmath>

Mandelbulb::Mandelbulb(const Parameters& params)
:ImplicitSurface(params)
{
	myPower = params.getInt("power", 5);
	myMaxIter = params.getInt("maxIter", 30);
}

Mandelbulb::~Mandelbulb()
{
}

//Normal3d Mandelbulb::normal(const Point3d& intersection) const //, const Normal3d& defaultNormal) const
//{
//	const Vector3d v1 = Vector3d(1.0, -1.0, -1.0);
//	const Vector3d v2 = Vector3d(-1.0, -1.0, 1.0);
//	const Vector3d v3 = Vector3d(-1.0, 1.0, -1.0);
//	const Vector3d v4 = Vector3d(1.0, 1.0, 1.0);
//	real eps = 1e-3;
//
//	Normal3d normal = v1 * estimateDistance(intersection + v1 * eps)+
//		v2 * estimateDistance(intersection + v2 * eps) +
//		v3 * estimateDistance(intersection + v3 * eps) +
//		v4 * estimateDistance(intersection + v4 * eps);
//	real length = normal.norm();
//	normal.normalize();
//	Normal3d defaultNormal(0.f);
//
//	//if (length == 0) std::cout << "longueur nulle" << std::endl;
//	return length > 0 ? normal : defaultNormal;
//}

//void Mandelbulb::getDifferentialGeometry(DifferentialGeometry& trueGeometry,
//	DifferentialGeometry& shadingGeometry, Intersection& inter)
//{
//	Normal3d computedNormal = normal(inter.myPoint);
//	if (computedNormal.norm() == (real) 0.)
//		computedNormal = -inter.myRay.direction();
//	trueGeometry = DifferentialGeometry(myObjectToWorld->transform(computedNormal));
//	shadingGeometry = trueGeometry;
//}

//bool Mandelbulb::intersection(const Ray& ray, real& t, Point2d& uv)
//{
//	uv = Point2d(0.f);
//	real epsilonDist = 1e-5;
//	t = ray.myMinT;
//	//int iter = 0;
//	while (t < ray.myMaxT)// && iter < 30)
//	{
//	
//		real dist = (estimateDistance(ray.getPointAt(t)));
//		
//		//if (dist < 0) std::cout << "??" << std::endl;
//		if (/*dist > 0 &&*/ dist < epsilonDist)
//		{
//			return true;
//		}	
//		t += dist;
//		//if(dist < 0)
//		//	t += 2 * dist;
//		//else
//			
//
//		//iter++;
//		//std::cout << dist << std::endl;
//	}
//
//	return false;
//}

real Mandelbulb::estimateDistance(const Point3d& from) const 
{
	//const float bailout = 2.0f;
	//const float Power = (float)myPower;
	//Point3d z = from;
	//float dr = 1.0;
	//float r = 0.0;
	//for (int i = 0; i < myMaxIter; i++) {
	//	r = (z - Point3d(0, 0, 0)).norm();
	//	if (r>bailout) break;

	//	// convert to polar coordinates
	//	float theta = acos(z.z() / r);
	//	float phi = atan2(z.y(), z.x());
	//	dr = pow(r, Power - 1.0)*Power*dr + 1.0;

	//	// scale and rotate the point
	//	float zr = pow(r, Power);
	//	theta = theta*Power;
	//	phi = phi*Power;

	//	// convert back to cartesian coordinates
	//	z = zr*Point3d(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
	//	z += from;
	//}
	//return 0.5*log(r)*r / dr;

	real x0 = from.x(), y0 = from.y(), z0 = from.z();
	real x = x0, y = y0, z = z0;
	real dr = 1.f;
	real R = 0.f;
	real maxNorm = 2.f;

	for (int i = 0; i < myMaxIter; i++)
	{
		real t = std::atan2(y, x);
		real p = std::atan2(z, std::sqrt(x * x + y * y));
		R = std::sqrt(x * x + y * y + z * z);

		if (R > maxNorm)
			break;

		real Rq = std::pow(R, myPower);
		real qp = myPower * p;
		real qt = myPower * t;
		real sinQp = std::sin(qp);
		x = Rq * sinQp * std::cos(qt) + x0;
		y = Rq * sinQp * std::sin(qt) + y0;
		z = Rq * std::cos(qp) + z0;
		dr = myPower * std::pow(R, myPower - 1) * dr + 1;
	}
	
	return 0.5 * R * std::log(R) / dr;
}

RT_REGISTER_TYPE_WITH_FACTORY(Mandelbulb, IPrimitive, GeometricShapeFactory)