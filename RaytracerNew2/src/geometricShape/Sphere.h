#pragma once
#include "GeometricShape.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief Sphere shape
///////////////////////////////////////////////////////////////////////////////
class Sphere : public GeometricShape
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor.
	///////////////////////////////////////////////////////////////////////////
	Sphere(const Parameters& params);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor.
	///////////////////////////////////////////////////////////////////////////
	~Sphere(void);

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Defines the sphere radius.
	///////////////////////////////////////////////////////////////////////////
	void setRadius(real r) {myRadius = r;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns the sphere radius.
	///////////////////////////////////////////////////////////////////////////
	real getRadius() const {return myRadius;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns the local sphere bounding box.
	///////////////////////////////////////////////////////////////////////////
	virtual BoundingBox getLocalBoundingBox() const override
	{
		return BoundingBox(Point3d(-myRadius, -myRadius, -myRadius),
						   Point3d(myRadius, myRadius, myRadius));
	}

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Computes the normal vector at the given point
	///////////////////////////////////////////////////////////////////////////
	Normal3d normal(const Point3d& intersection) const;
	
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
	virtual bool intersection(const Ray& ray, real& t, Point2d& uv) override; //DifferentialGeometry& trueGeom, DifferentialGeometry& shadingGeom, bool shadowRay = false) override;

	void getDifferentialGeometry(DifferentialGeometry& trueGeometry,
		DifferentialGeometry& shadingGeometry, Intersection& inter) override;

	void sample(const Point2d& p, Point3d& sampled, Normal3d& normal);

	//_
	//	(| \) <- sphere, with a right angle between (center to sphere inter, solid angle extremity)
	//	\|t/  <- solid angle formed at pFrom, with theta (t)
	// (sin theta)^2 = r^2 / d^2 (d is the distance from sphere center to "bsdf" inter)
	//-> cos theta = ...
	real pdf(const Point3d& pFrom, const Point3d& sampled, const Normal3d& normal);

	real surfaceArea()
	{
		//Consider scale factor ??????!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		return 4.f * math::PI * myRadius * myRadius * myRadius;
	}

protected:
	real myRadius;
	Point3d myCenter;
};

