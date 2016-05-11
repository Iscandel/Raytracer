#include "Sphere.h"
#include "Tools.h"
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
	myRadius = params.getDouble("radius", 1.);
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
Vector3d Sphere::normal(const Point3d& intersection) const
{
	Point3d localPoint = myObjectToWorld->inv().transform(intersection);

	Vector3d normalSphere = localPoint - myCenter;
	normalSphere.normalize();
	return normalSphere;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
bool Sphere::intersection(const Ray& ray, double& t, Point2d& uv) //DifferentialGeometry& trueGeom,	DifferentialGeometry& shadingGeom, bool shadowRay)
{
	//Transform the ray to an object space ray
	Ray localRay = myObjectToWorld->inv().transform(ray);
	//localRay = ray;
	Vector3d sphereToOrig(myCenter, localRay.myOrigin);
	double a = 1;//dx * dx + dy * dy + dz * dz;
	double b = 2 * localRay.direction().dot(sphereToOrig);
	double c = sphereToOrig.dot(sphereToOrig) - myRadius * myRadius;

	double delta = b * b - 4 * a * c;

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
		double t1 = (-b + sqrt(delta)) / (2 * a);
		double t2 = (-b - sqrt(delta)) / (2 * a);
	
		if (t1 <= 0) t = t2;
		if (t2 <= 0) t = t1;
		if (t1 > 0 && t2 > 0)
			t = std::min(t1, t2);

		//trueGeom.myN = normal(ray.myOrigin + t * ray.direction());
		//shadingGeom = trueGeom;

		bool intersect = t >= ray.myMinT && t <= ray.myMaxT;
		if (intersect)
		{
			//UV mapping from spherical coords
			Vector3d normalizedInterPoint = ray.getPointAt(t).normalized();
			uv.x() = sphericalPhiFromCartesian(normalizedInterPoint) / (2 * tools::PI);
			uv.y() = sphericalThetaFromCartesian(normalizedInterPoint) / tools::PI;
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
	trueGeometry.myN = normal(inter.myPoint);
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

	//double right = std::sqrt(p.y() * (1 - p.y()));
	//sampled.x() = center.x() + 2 * myRadius * std::cos(2 * tools::PI * p.x()) * right;
	//sampled.y() = center.y() + 2 * myRadius * std::sin(2 * tools::PI * p.x()) * right;
	//sampled.z() = center.z() + myRadius * (1 - 2 * p.y());
	//
	//sampled = myObjectToWorld->transform(sampled);
	//n = normal(sampled);

	double theta = 2. * tools::PI * p.x();
	double phi = std::acos(2 * p.y() - 1.);

	double u = std::cos(phi);
	double sqrtVal = std::sqrt(1 - u * u);
	sampled.x() = sqrtVal * std::cos(theta);
	sampled.y() = sqrtVal * std::sin(theta);
	sampled.z() = u;

	sampled *= myRadius;

	sampled = myObjectToWorld->transform(sampled);
	n = normal(sampled);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
// Pdf with respect to the solid angle
//    _
//	(| \) <- sphere, with a right angle between (center to sphere inter, solid angle extremity)
//	\|t/  <- solid angle formed at pFrom, with theta (t)
// (sin theta)^2 = r^2 / d^2 (d is the distance from sphere center to "bsdf" inter)
//-> cos theta = ...
double Sphere::pdf(const Point3d& pFrom, const Point3d& sampled, const Normal3d& n)
{
	Point3d center = myObjectToWorld->transform(Point3d(0., 0., 0.));
	Vector3d interToCenter = pFrom - center;
	double squaredDistance = interToCenter.squaredNorm();//interToCenter.x() * interToCenter.x() + interToCenter.y() * interToCenter.y() + interToCenter.z() * interToCenter.z();

	if (n.dot(interToCenter) < 0.)
		return 0.;

	//If sampled point is inside the sphere, use a uniform weight
	if (squaredDistance - myRadius * myRadius < tools::EPSILON)
		return GeometricShape::pdf(pFrom, sampled, n);

	double sinTheta2 = myRadius * myRadius / squaredDistance;
	double cosTheta = std::sqrt(std::max(0., 1 - sinTheta2));

	return Mapping::uniformConePdf(cosTheta);

}

RT_REGISTER_TYPE_WITH_FACTORY(Sphere, IPrimitive, GeometricShapeFactory)
