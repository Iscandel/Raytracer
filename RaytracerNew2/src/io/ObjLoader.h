#pragma once

#include "IMeshLoader.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief Class dedicated to load OBJ files.
///////////////////////////////////////////////////////////////////////////////
class ObjLoader : IMeshLoader
{
public:
	ObjLoader();
	~ObjLoader();

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Reads a new model file.
	/// 
	/// \param vertices a vector containing the returned vertex positions.
	/// \param normals a vector containing the returned normal values.
	/// \param UVs a vector containing the returned UV values.
	/// \param indices a N x 1vector containing the indices of vertices, normals ,UV
	/// for each face (1...N)
	/// \param params Some parameters.
	///
	/// \return true if loading was ok.
	///////////////////////////////////////////////////////////////////////////////
	bool read(std::vector<Point3d, Eigen::aligned_allocator<Point3d>>& vertices,
		std::vector<Normal3d, Eigen::aligned_allocator<Normal3d>>& normals,
		std::vector<Point2d, Eigen::aligned_allocator<Point2d>>& UVs,
		std::vector<int>& indices, 
		const Parameters& params,
		std::vector<std::pair<int, BSDF::ptr>>* BSDFAndTriangleIndexTimes3) override;

protected:
	void addBSDF(std::map<std::string, BSDF::ptr>& map,
		const std::string& name, int illum, real Ns, real Ni,
		Texture::ptr d, Texture::ptr Kd, Texture::ptr Ks, Texture::ptr bumpMap, 
		bool bumpIsNormal);

	std::map<std::string, BSDF::ptr> parseMtl(const std::string& path, const Parameters& params);

protected:
	std::map<std::string, BSDF::ptr> myBSDFByName;
	//std::vector<std::pair<int, BSDF::ptr>> myBSDFAndTriangleIndexTimes3;
};

