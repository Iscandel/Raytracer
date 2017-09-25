//#pragma once
//#include "GeometricShape.h"
//class Cube :
//	public GeometricShape
//{
//public:
//	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
//public:
//	Cube(const Parameters& params);
//	~Cube();
//
//	///////////////////////////////////////////////////////////////////////////
//	/// \brief Returns the local sphere bounding box.
//	///////////////////////////////////////////////////////////////////////////
//	virtual BoundingBox getLocalBoundingBox() const override;
//
//public:
//	///////////////////////////////////////////////////////////////////////////
//	/// \brief Computes the normal vector at the given point
//	///////////////////////////////////////////////////////////////////////////
//	Normal3d normal(const Point3d& intersection) const;
//
//	///////////////////////////////////////////////////////////////////////////
//	/// \brief Determines if the given ray intersects the shape.
//	///
//	/// \param ray The considered ray.
//	///
//	/// \param t Out parameter indicating the parametric distance of the 
//	/// intersection point to the ray origin, if an intersection was found.
//	///
//	/// \param shadowRay Indicates whether the ray is a shadow ray (faster 
//	/// intersection routines) of not.
//	///////////////////////////////////////////////////////////////////////////
//	virtual bool intersection(const Ray& ray, double& t, Point2d& uv) override;
//
//	void getDifferentialGeometry(DifferentialGeometry& trueGeometry,
//		DifferentialGeometry& shadingGeometry, Intersection& inter) override;
//
//	void sample(const Point2d& p, Point3d& sampled, Normal3d& normal);
//
//	double surfaceArea()
//	{
//		return mySurface;
//	}
//
//protected:
//	Point3d myMin;
//	Point3d myMax;
//
//	double mySurface;
//	double myInvSurface;
//};
//
