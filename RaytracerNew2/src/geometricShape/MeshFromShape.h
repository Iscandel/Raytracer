#pragma once
#include "core/Geometry.h"

#include <memory>
#include <vector>

#include <Eigen/StdVector>

class MeshFromShape
{
public:
	typedef std::shared_ptr<MeshFromShape> ptr;

	virtual void computeMeshObject(std::vector<Point3d, Eigen::aligned_allocator<Point3d>>& vertices,
		std::vector<Normal3d, Eigen::aligned_allocator<Normal3d>>& normals,
		std::vector<Point2d, Eigen::aligned_allocator<Point2d>>& UVs,
		std::vector<int>& indices) = 0;
};

