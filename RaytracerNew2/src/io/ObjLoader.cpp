#include "ObjLoader.h"

#include "tools/Logger.h"
#include "tools/Tools.h"
#include "core/Transform.h"

#include <fstream>
#include <unordered_map>

ObjLoader::ObjLoader()
{
}


ObjLoader::~ObjLoader()
{
}



struct VertexObj
{
	VertexObj()
	{
		indPos = 0;
		indN = 0;
		indUV = 0;
	}

	VertexObj(const std::string& objLine)
	{
		//VertexObj();
		//if (objLine.find("//") != std::string::npos)
		//{
		//	throw std::runtime_error("erreur obj");
		//}
		//else
		{
			std::istringstream s(objLine);
			std::vector<std::string> tokens;
			for (std::string token; std::getline(s, token, '/'); )
				tokens.push_back(token);

			if (tokens.size() < 1 || tokens.size() > 3)
				ILogger::log(ILogger::ERRORS) << "Illegal vertex value : " << objLine << "\n";

			indPos = tools::stringToNum<int32_t>(tokens[0]);

			if (tokens.size() > 1 && !tokens[1].empty())
				indUV = tools::stringToNum<int32_t>(tokens[1]);

			if (tokens.size() > 2 && !tokens[2].empty())
				indN = tools::stringToNum<int32_t>(tokens[2]);
		}
	}

	bool operator==(const VertexObj &v) const 
	{
		return v.indPos == indPos && v.indN == indN && v.indUV == indUV;
	}

	int32_t indPos;
	int32_t indN;
	int32_t indUV;
};

/// Hash function for VertexObj
struct VertexObjHash : std::unary_function<VertexObjHash, size_t> {
	std::size_t operator()(const VertexObj &v) const {
		size_t hash = std::hash<uint32_t>()(v.indPos);
		hash = hash * 37 + std::hash<uint32_t>()(v.indUV);
		hash = hash * 37 + std::hash<uint32_t>()(v.indN);
		return hash;
	}
};

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
bool ObjLoader::read(std::vector<Point3d, Eigen::aligned_allocator<Point3d>>& positions,
					 std::vector<Normal3d, Eigen::aligned_allocator<Normal3d>>& normals,
					 std::vector<Point2d, Eigen::aligned_allocator<Point2d>>& UVs,
					 std::vector<int>& indices, 
					 const Parameters& params)
{
	typedef std::unordered_map<VertexObj, uint32_t, VertexObjHash> VertexMap;

	Transform::ptr transform = params.getTransform("toWorld", std::shared_ptr<Transform>(new Transform));

	std::string filename = params.getString("path", "");

	ILogger::log(ILogger::ALL) << "Loading " << filename << "\n";

	std::ifstream is(filename);
	if (is.fail())
	{
		ILogger::log(ILogger::ERRORS) << "File " << filename << " cannot be open.\n";
		return false;
	}

	std::vector<Point3d, Eigen::aligned_allocator<Point3d>> tmpPositions;
	std::vector<Point2d, Eigen::aligned_allocator<Point2d>> tmpTextcoords;
	std::vector<Normal3d, Eigen::aligned_allocator<Normal3d>> tmpNormalsVect;
	std::vector<VertexObj, Eigen::aligned_allocator<VertexObj>> vertices;
	//std::vector<int> indices;
	VertexMap vertexMap;

	std::string line;
	while (std::getline(is, line))
	{
		std::istringstream sLine(line);

		std::string type;
		sLine >> type;

		if (type == "v") {
			Point3d pos;
			sLine >> pos.x() >> pos.y() >> pos.z();
			pos = transform->transform(pos);
			//m_bbox.expandBy(p);
			tmpPositions.push_back(pos);
		}
		else if (type == "vn")
		{
			Normal3d n;
			sLine >> n.x() >> n.y() >> n.z();
			n = transform->transform(n);
			tmpNormalsVect.push_back(n);
		}
		else if (type == "vt")
		{
			Point2d text;
			sLine >> text.x() >> text.y();
			tmpTextcoords.push_back(text);
		}
		else if (type == "f")
		{
			std::string lineVert[5];
			Point2d text;
			sLine >> lineVert[0] >> lineVert[1] >> lineVert[2] >> lineVert[3] >> lineVert[4];

			VertexObj tmpVert[6];
			int nbVertices = 3;
			tmpVert[0] = VertexObj(lineVert[0]);
			tmpVert[1] = VertexObj(lineVert[1]);
			tmpVert[2] = VertexObj(lineVert[2]);

			//If not empty, this is a quad we have to split into two triangles
			if (!lineVert[3].empty())
			{
				//std::vector<std::string> polygonVert;
				//polygonVert.assign(tmpVert, tmpVert + 3);
				if (!lineVert[4].empty())
					ILogger::log() << "This mesh has polygonal faces.\n";
				tmpVert[3] = VertexObj(lineVert[3]);
				tmpVert[4] = tmpVert[0];
				tmpVert[5] = tmpVert[2];
				nbVertices = 6;
			}

			//Affect an index to face vertices
			for (int i = 0; i < nbVertices; i++)
			{
				const VertexObj &v = tmpVert[i];
				VertexMap::const_iterator it = vertexMap.find(v);
				if (it == vertexMap.end())
				{
					vertexMap[v] = vertices.size();
					indices.push_back((int)vertices.size());
					vertices.push_back(v);
				}
				else
				{
					//if this vertex already has an index, affect it for the current face
					indices.push_back(it->second);
				}
			}
		}
	}

	positions.resize(vertices.size());
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		if (vertices[i].indPos < 0)
			vertices[i].indPos = (int)tmpPositions.size() + vertices[i].indPos + 1;

		positions[i] = tmpPositions[vertices[i].indPos - 1];
	}

	if (!tmpNormalsVect.empty())
	{
		normals.resize(vertices.size());
		for (unsigned int i = 0; i < vertices.size(); i++)
		{
			if (vertices[i].indN < 0)
				vertices[i].indN = (int)tmpNormalsVect.size() + vertices[i].indN + 1;

			normals[i] = tmpNormalsVect[vertices[i].indN - 1];
		}
	}

	
	if (!tmpTextcoords.empty())
	{
		UVs.resize(vertices.size());
		for (unsigned int i = 0; i < vertices.size(); i++)
		{
			if (vertices[i].indUV < 0)
				vertices[i].indUV = (int)tmpTextcoords.size() + vertices[i].indUV + 1;

			UVs[i] = tmpTextcoords[vertices[i].indUV - 1];
		}
	}

	return true;
}