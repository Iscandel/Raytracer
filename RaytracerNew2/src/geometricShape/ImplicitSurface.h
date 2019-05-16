#pragma once
#include "GeometricShape.h"
class ImplicitSurface :
	public GeometricShape
{
public:
	ImplicitSurface(const Parameters& params);
	~ImplicitSurface();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Computes the normal vector at the given point
	///////////////////////////////////////////////////////////////////////////
	Normal3d normal(const Point3d& intersection) const;//, const Normal3d& defaultNormal) const;

	virtual real estimateDistance(const Point3d& from) const = 0;

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
	bool intersection(const Ray& ray, real& t, Point2d& uv) override;

	void getDifferentialGeometry(DifferentialGeometry& trueGeometry,
		DifferentialGeometry& shadingGeometry, Intersection& inter) override;
};

