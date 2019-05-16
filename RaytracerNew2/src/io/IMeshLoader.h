#pragma once

#include "core/Geometry.h"
#include "core/Parameters.h"

#include <vector>

///////////////////////////////////////////////////////////////////////////////
/// \brief Abstract class for the model file readers.
///////////////////////////////////////////////////////////////////////////////
class IMeshLoader
{
public:
	IMeshLoader();
	~IMeshLoader();

	//static IMeshLoader* getLoader(const Parameters& params)
	//{
	//	static ObjLoader * loader = new ObjLoader;

	//	return loader;
	//}

	virtual bool read(std::vector<Point3d, Eigen::aligned_allocator<Point3d>>& vertices,
		std::vector<Normal3d, Eigen::aligned_allocator<Normal3d>>& normals,
		std::vector<Point2d, Eigen::aligned_allocator<Point2d>>& UVs,
		std::vector<int>& indices, 
		const Parameters& params) = 0;
};

