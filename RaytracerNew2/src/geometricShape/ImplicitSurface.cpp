#include "ImplicitSurface.h"

#include "factory/GeometricShapeFactory.h"
#include "core/Intersection.h"

#include <cmath>

ImplicitSurface::ImplicitSurface(const Parameters& params)
:GeometricShape(params)
{
}


ImplicitSurface::~ImplicitSurface()
{
}

Normal3d ImplicitSurface::normal(const Point3d& intersection) const //, const Normal3d& defaultNormal) const
{
	const Vector3d v1 = Vector3d(1.0, -1.0, -1.0);
	const Vector3d v2 = Vector3d(-1.0, -1.0, 1.0);
	const Vector3d v3 = Vector3d(-1.0, 1.0, -1.0);
	const Vector3d v4 = Vector3d(1.0, 1.0, 1.0);
	real eps = 1e-3;

	Normal3d normal = v1 * estimateDistance(intersection + v1 * eps) +
		v2 * estimateDistance(intersection + v2 * eps) +
		v3 * estimateDistance(intersection + v3 * eps) +
		v4 * estimateDistance(intersection + v4 * eps);
	real length = normal.norm();
	normal.normalize();
	Normal3d defaultNormal(0.f);

	//if (length == 0) std::cout << "longueur nulle" << std::endl;
	return length > 0 ? normal : defaultNormal;
}

void ImplicitSurface::getDifferentialGeometry(DifferentialGeometry& trueGeometry,
	DifferentialGeometry& shadingGeometry, Intersection& inter)
{
	Normal3d computedNormal = normal(inter.myPoint);
	if (computedNormal.norm() == (real) 0.)
		computedNormal = -inter.myRay.direction();
	trueGeometry = DifferentialGeometry(myObjectToWorld->transform(computedNormal));
	shadingGeometry = trueGeometry;
}

bool ImplicitSurface::intersection(const Ray& ray, real& t, Point2d& uv)
{
	uv = Point2d(0.f);
	real epsilonDist = 1e-5;
	t = ray.myMinT;
	//int iter = 0;
	while (t < ray.myMaxT)// && iter < 30)
	{

		real dist = (estimateDistance(ray.getPointAt(t)));

		//if (dist < 0) std::cout << "??" << std::endl;
		if (/*dist > 0 &&*/ dist < epsilonDist)
		{
			return true;
		}
		t += dist;
		//if(dist < 0)
		//	t += 2 * dist;
		//else


		//iter++;
		//std::cout << dist << std::endl;
	}

	return false;
}
