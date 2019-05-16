#pragma once
#include "ImplicitSurface.h"

class Mandelbulb :
	public ImplicitSurface
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor.
	///////////////////////////////////////////////////////////////////////////
	Mandelbulb(const Parameters& params);
	~Mandelbulb();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns the local sphere bounding box.
	///////////////////////////////////////////////////////////////////////////
	virtual BoundingBox getLocalBoundingBox() const override
	{
		return BoundingBox(Point3d(-2., -2., -2.), Point3d(2., 2., 2.));
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Computes the normal vector at the given point
	///////////////////////////////////////////////////////////////////////////
	//Normal3d normal(const Point3d& intersection) const;//, const Normal3d& defaultNormal) const;

	real estimateDistance(const Point3d& from) const override;

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
	//virtual bool intersection(const Ray& ray, real& t, Point2d& uv) override;

	//void getDifferentialGeometry(DifferentialGeometry& trueGeometry,
	//	DifferentialGeometry& shadingGeometry, Intersection& inter) override;

	void sample(const Point2d& p, Point3d& sampled, Normal3d& normal) override { throw std::exception("Sampling Mandelbulb not implemented"); }

	real surfaceArea()
	{
		throw std::exception("SurfaceArea() Mandelbulb not implemented");
	}

protected:
	int myPower;
	int myMaxIter;
};

