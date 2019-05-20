#pragma once

#include "core/CDF.h"
#include "GeometricShape.h"
#include "tools/Tools.h"

#include <Eigen/StdVector>
#include <memory>
#include <vector>

class GeometricShape;

#include "tools/Rng.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief Data structure storing geometric informations about triangles that
/// compound the mesh
///////////////////////////////////////////////////////////////////////////////
class Mesh : public ISampledShape, public std::enable_shared_from_this<Mesh>
{
public:
	typedef std::shared_ptr<Mesh> ptr;
public:
	Mesh(const std::string& name, 
		 const std::vector<Point3d, Eigen::aligned_allocator<Point3d>>& vert,
		 const std::vector<Normal3d, Eigen::aligned_allocator<Normal3d>>& normals,
		 const std::vector<Point2d, Eigen::aligned_allocator<Point2d>>& myUVs,
		 const std::vector<int>& indices);
	~Mesh();

	void sample(const Point2d& p, Point3d& sampled, Normal3d& normal) override;

	real pdf(const Point3d& p, const Point3d& sampled, const Normal3d& normal) override;

	real surfaceArea() override {
		return mySurfaceArea;
	}

	std::vector<Point3d, Eigen::aligned_allocator<Point3d>> myVertices;
	std::vector<Normal3d, Eigen::aligned_allocator<Normal3d>> myNormals;
	std::vector<Point2d, Eigen::aligned_allocator<Point2d>> myUVs;
	std::vector<int> myIndices;

	void addLightTriangle(std::shared_ptr<GeometricShape> triangle);

	void initialize() {
		myCDF.normalize();
	}

protected:
	//Vector used to store light associated to the mesh (if any)
	std::vector<std::shared_ptr<GeometricShape>> myLightTriangles;
	//Rng rng;
	//CDF
	CDF myCDF;

	real mySurfaceArea;
};




