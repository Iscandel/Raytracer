#pragma once
#include "IMeshLoader.h"

struct PLYHeader
{
	std::string type;
	std::string format;
	int vertexNumber;
	int faceNumber;
};
class PLYLoader :
	public IMeshLoader
{
public:
	PLYLoader();
	~PLYLoader();

	PLYHeader readHeader(std::ifstream& file);

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
		const Parameters& params) override;
};

