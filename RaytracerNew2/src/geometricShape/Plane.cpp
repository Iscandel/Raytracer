#include "Plane.h"

#include "core/Intersection.h"
#include "factory/GeometricShapeFactory.h"
#include "factory/ObjectFactoryManager.h"

Plane::Plane(const Parameters& params)
:myMin(-1.,-1.,0.)
,myMax(1.,1.,0.)
{
	myObjectToWorld = params.getTransform("toWorld", Transform::ptr(new Transform));
	Point3d v2(-1, 1, 0.);
	Point3d v3(1.f, -1., 0.);
	Vector3d side1 = myObjectToWorld->transform(v2) - myObjectToWorld->transform(myMin);
	Vector3d side2 = myObjectToWorld->transform(v3) - myObjectToWorld->transform(myMin);
	mySurface = side1.norm() * side2.norm();
	myInvSurface = 1.f / mySurface;
}


Plane::~Plane()
{
}

///////////////////////////////////////////////////////////////////////////
/// \brief Returns the local sphere bounding box.
///////////////////////////////////////////////////////////////////////////
BoundingBox Plane::getLocalBoundingBox() const
{
	//Point3d v2(-1, 1, 0.);
	//Point3d v3(1., -1., 0.);
	//v2 = v2);
	//v3 = myObjectToWorld->transform(v3);
	//
	//BoundingBox box(myObjectToWorld->transform(myMin));
	//BoundingBox::unionBox(box, myObjectToWorld->transform(v2));
	//BoundingBox::unionBox(box, myObjectToWorld->transform(v3));
	//BoundingBox::unionBox(box, myObjectToWorld->transform(myMax));
	//return box;

	return BoundingBox(myMin, myMax);
}

///////////////////////////////////////////////////////////////////////////
/// \brief Computes the normal vector at the given point
///////////////////////////////////////////////////////////////////////////
Normal3d Plane::normal(const Point3d&) const
{
	return Normal3d(0., 0., 1.);
}

///////////////////////////////////////////////////////////////////////////
/// \brief Determines if the given ray intersects the shape.
///
/// \param ray The considered ray.
///
/// \param t Out parameter indicating the parametric distance of the 
/// intersection point to the ray origin, if an intersection was found.
///
/// \param shadowRay Indicates whether the ray is a shadow ray (faster 
/// intersection routines) of not.
///////////////////////////////////////////////////////////////////////////
bool Plane::intersection(const Ray& _ray, real& t, Point2d& uv)
{
	Ray ray(myObjectToWorld->inv().transform(_ray));

	//plane is at z=0. z = ray.orig.z + k * dir.z
	t = -ray.myOrigin.z() / ray.direction().z();
	
	if (t < ray.myMinT || t > ray.myMaxT)
		return false;

	Point3d p = ray.getPointAt(t);

	if (p.x() < myMin.x() || p.x() > myMax.x() || p.y() < myMin.y() || p.y() > myMax.y())
		return false;

	uv.x() = (p.x() + 1) / 2.f;
	uv.y() = (p.y() + 1) / 2.f;

	return true;
}

void Plane::getDifferentialGeometry(DifferentialGeometry& trueGeometry,
	DifferentialGeometry& shadingGeometry, Intersection& inter)
{
	trueGeometry = DifferentialGeometry(myObjectToWorld->transform(normal(inter.myPoint)).normalized());
	Vector3d dpdu(1., 0, 0);
	Vector3d dpdv(0, 1, 0);
	trueGeometry.dpdu = myObjectToWorld->transform(dpdu).normalized();
	trueGeometry.dpdv = myObjectToWorld->transform(dpdv).normalized();

	shadingGeometry = trueGeometry;
}

void Plane::sample(const Point2d& p, Point3d& sampled, Normal3d& n)
{
	sampled = myObjectToWorld->transform(Point3d(1 - 2 * p.x(), 1 - 2 * p.y(), 0));
	n = myObjectToWorld->transform(normal(sampled)).normalized();
}

RT_REGISTER_TYPE_WITH_FACTORY(Plane, IPrimitive, GeometricShapeFactory)