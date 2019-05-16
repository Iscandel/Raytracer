#include "PLYLoader.h"

#include "tools/Logger.h"
#include "tools/Tools.h"

#include <fstream>

PLYLoader::PLYLoader()
{
}


PLYLoader::~PLYLoader()
{
}

PLYHeader PLYLoader::readHeader(std::ifstream& file)
{
	PLYHeader header;

	std::string line;
	
	std::getline(file, line);
	header.type = line;
	while (line != "end_header")
	{
		if (line.find("comment") != std::string::npos)
			continue;
		else if (line.find("element") != std::string::npos)
		{
			std::istringstream s(line);
			std::vector<std::string> tokens;
			for (std::string token; std::getline(s, token, ' '); )
				tokens.push_back(token);

			if (tokens.size() < 1 || tokens.size() > 3)
				ILogger::log(ILogger::ERRORS) << "Illegal vertex value : " << line << "\n";

			if (tokens[1] == "vertex")
			{
				header.vertexNumber = tools::stringToNum<int>(tokens[2]);
			}
			else if (tokens[1] == "face")
			{
				header.faceNumber = tools::stringToNum<int>(tokens[2]);
			}
		}
		std::getline(file, line);
	}


	return header;
}

bool PLYLoader::read(std::vector<Point3d, Eigen::aligned_allocator<Point3d>>& vertices, 
	std::vector<Normal3d, Eigen::aligned_allocator<Normal3d>>& normals, 
	std::vector<Point2d, Eigen::aligned_allocator<Point2d>>& UVs, 
	std::vector<int>& indices, 
	const Parameters & params)
{
	Transform::ptr transform = params.getTransform("toWorld", std::shared_ptr<Transform>(new Transform));

	std::string filename = params.getString("path", "");

	ILogger::log(ILogger::ALL) << "Loading " << filename << "\n";

	std::ifstream file(filename);
	if (file.fail())
	{
		ILogger::log(ILogger::ERRORS) << "File " << filename << " cannot be open.\n";
		return false;
	}

	//std::ifstream file("./data/bun_zipper.ply");

	PLYHeader header = readHeader(file);

	//ply
	//	format ascii 1.0
	//	comment zipper output
	//	element vertex 453
	//	property float x
	//	property float y
	//	property float z
	//	property float confidence
	//	property float intensity
	//	element face 948
	//	property list uchar int vertex_indices
	//	end_header

//	std::vector<Point3d, Eigen::aligned_allocator<Point3d>> vertices;
	for (auto i = 0; i < header.vertexNumber; i++)
	{
		std::string line;
		std::getline(file, line);
		std::istringstream stream(line);
		Point3d point;
		stream >> point.x() >> point.y() >> point.z();
		point = transform->transform(point),
		vertices.push_back(point);
	}

	for (auto i = 0; i < header.faceNumber; i++)//948; i++)
	{
		std::string line;
		std::getline(file, line);
		std::istringstream stream(line);
		real no;
		real tmp[3];
		stream >> no >> tmp[0] >> tmp[1] >> tmp[2];
		Point3d verticeTriangle[3];
		verticeTriangle[0] = vertices[tmp[0]];
		verticeTriangle[1] = vertices[tmp[1]];
		verticeTriangle[2] = vertices[tmp[2]];

		//Triangle* triangle = new Triangle(10., 0.7, 0.8, 100, Color(i % 255, (i * 3) % 255, (i * 5) % 255), verticeTriangle);
		//IPrimitive::ptr primitive(std::make_shared<SimplePrimitive>(GeometricShape::ptr(triangle), BSDF::ptr()));
		//addPrimitive(primitive);
	}

	return true;

}