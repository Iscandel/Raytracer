#pragma once
#include "geometricShape/MeshFromShape.h"
#include "core/Parameters.h"

class Cube :
	public MeshFromShape
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
public:
	Cube(const Parameters& params);
	~Cube();

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
//	virtual bool intersection(const Ray& ray, real& t, Point2d& uv) override;
//
//	void getDifferentialGeometry(DifferentialGeometry& trueGeometry,
//		DifferentialGeometry& shadingGeometry, Intersection& inter) override;
//
//	void sample(const Point2d& p, Point3d& sampled, Normal3d& normal);
//
//	real surfaceArea()
//	{
//		return mySurface;
//	}

	void computeMeshObject(std::vector<Point3d, Eigen::aligned_allocator<Point3d>>& vertices,
		std::vector<Normal3d, Eigen::aligned_allocator<Normal3d>>& normals,
		std::vector<Point2d, Eigen::aligned_allocator<Point2d>>& UVs,
		std::vector<int>& indices) override;

protected:
	Transform::ptr myTransform;
};
//
