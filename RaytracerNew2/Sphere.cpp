#include "Sphere.h"
#include "Math.h"
#include "Intersection.h"
#include "GeometricShapeFactory.h"
#include "Mapping.h"
#include "ObjectFactoryManager.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Sphere::Sphere(const Parameters& params)
:myCenter(Point3d(0., 0., 0.))
, myRadius(0.)
{
	myRadius = params.getReal("radius", 1.);
	myObjectToWorld = params.getTransform("toWorld", Transform::ptr(new Transform));
	//myCenter = myObjectToWorld->
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Sphere::~Sphere(void)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Normal3d Sphere::normal(const Point3d& intersection) const
{
	Point3d localPoint = myObjectToWorld->inv().transform(intersection);

	Normal3d normalSphere = localPoint - myCenter;
	normalSphere.normalize();
	return normalSphere;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
bool Sphere::intersection(const Ray& ray, real& t, Point2d& uv) //DifferentialGeometry& trueGeom,	DifferentialGeometry& shadingGeom, bool shadowRay)
{
	//Transform the ray to an object space ray
	Ray localRay = myObjectToWorld->inv().transform(ray);
	//localRay = ray;
	Vector3d sphereToOrig(myCenter, localRay.myOrigin);
	real a = 1;//dx * dx + dy * dy + dz * dz;
	real b = 2 * localRay.direction().dot(sphereToOrig);
	real c = sphereToOrig.dot(sphereToOrig) - myRadius * myRadius;

	real delta = b * b - 4 * a * c;

	//// screen plane in world space coordinates
	//m_WX1 = -4, m_WX2 = 4, m_WY1 = m_SY = 3, m_WY2 = -3;
	//// calculate deltas for interpolation
	//m_DX = (m_WX2 - m_WX1) / m_Width;
	//m_DY = (m_WY2 - m_WY1) / m_Height;

	if(delta < 0)
	{
		return false;
	}
	else
	{
		real t1 = (-b + sqrt(delta)) / (2 * a);
		real t2 = (-b - sqrt(delta)) / (2 * a);
	
		//if (t1 <= 0) t = t2;
		//if (t2 <= 0) t = t1;
		//if (t1 > 0 && t2 > 0)
		//	t = std::min(t1, t2);
		//bool intersect = t >= ray.myMinT && t <= ray.myMaxT;

		real tNear = t1;
		real tFar = t2;
		if (tNear > tFar)
			std::swap(tNear, tFar);

		//trueGeom.myN = normal(ray.myOrigin + t * ray.direction());
		//shadingGeom = trueGeom;

		bool intersect = false;

		//if (!(tNear >= ray.myMinT && tFar <= ray.myMaxT))
		//	return false;

		if (tNear <= ray.myMinT)
		{
			if (tFar > ray.myMinT && tFar < ray.myMaxT)
			{
				t = tFar;
				intersect = true;
			}
		}
		else if (tNear > ray.myMinT && tNear < ray.myMaxT)
		{
			intersect = true;
			t = tNear;
		}

		if (intersect)
		{
			//UV mapping from spherical coords
			Vector3d normalizedInterPoint = ray.getPointAt(t).normalized();
			uv.x() = sphericalPhiFromCartesian(normalizedInterPoint) / (2 * math::PI);
			uv.y() = sphericalThetaFromCartesian(normalizedInterPoint) / math::PI;
			return true;
		}
		return false;

		//return t >= ray.myMinT && t <= ray.myMaxT;
		
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void Sphere::getDifferentialGeometry(DifferentialGeometry & trueGeometry, DifferentialGeometry & shadingGeometry, Intersection & inter)
{
	trueGeometry = DifferentialGeometry(myObjectToWorld->transform(normal(inter.myPoint)));
	shadingGeometry = trueGeometry;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
//We would need the 3d point 'pFrom' to check the dot(normal, pFrom) and correct the sampled point if dot < 0 (avoid wasting sample)
//Algorithm from http://mathworld.wolfram.com/SpherePointPicking.html
//Nb : We should sample the spherical cap
void Sphere::sample(const Point2d& p, Point3d& sampled, Normal3d& n)
{
	////Point3d center = myObjectToWorld->transform(Point3d(0., 0., 0.));
	//Point3d center = myCenter; //always 0, 0, 0

	//real right = std::sqrt(p.y() * (1 - p.y()));
	//sampled.x() = center.x() + 2 * myRadius * std::cos(2 * tools::PI * p.x()) * right;
	//sampled.y() = center.y() + 2 * myRadius * std::sin(2 * tools::PI * p.x()) * right;
	//sampled.z() = center.z() + myRadius * (1 - 2 * p.y());
	//
	//sampled = myObjectToWorld->transform(sampled);
	//n = normal(sampled);

	real theta = 2.f * math::PI * p.x();
	real phi = std::acos(2 * p.y() - 1.f);

	real u = std::cos(phi);
	real sqrtVal = std::sqrt(1 - u * u);
	sampled.x() = sqrtVal * std::cos(theta);
	sampled.y() = sqrtVal * std::sin(theta);
	sampled.z() = u;

	sampled *= myRadius;
	n = myObjectToWorld->transform(normal(sampled));
	n.normalize(); //new
	sampled = myObjectToWorld->transform(sampled);
	
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
// Pdf with respect to the solid angle
//    _
//	(| \) <- sphere, with a right angle between (center to sphere inter, solid angle extremity)
//	\|t/  <- solid angle formed at pFrom, with theta (t)
// (sin theta)^2 = r^2 / d^2 (d is the distance from sphere center to "bsdf" inter)
//-> cos theta = ...
real Sphere::pdf(const Point3d& pFrom, const Point3d& sampled, const Normal3d& n)
{
	//return 1. / (4 * tools::PI);
	Point3d center = myObjectToWorld->transform(Point3d(0.f, 0.f, 0.f));
	Vector3d interToCenter = pFrom - center;
	real squaredDistance = interToCenter.squaredNorm();//interToCenter.x() * interToCenter.x() + interToCenter.y() * interToCenter.y() + interToCenter.z() * interToCenter.z();

	if (n.dot(interToCenter) < 0.)
		return 0.;

	//If sampled point is inside the sphere, use a uniform weight
	if (squaredDistance - myRadius * myRadius < math::EPSILON)
		return GeometricShape::pdf(pFrom, sampled, n);

	real sinTheta2 = myRadius * myRadius / squaredDistance;
	real cosTheta = std::sqrt(std::max((real)0., 1 - sinTheta2));

	return Mapping::uniformConePdf(cosTheta);

}

RT_REGISTER_TYPE_WITH_FACTORY(Sphere, IPrimitive, GeometricShapeFactory)
