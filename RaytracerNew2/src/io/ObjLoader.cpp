#include "ObjLoader.h"

#include "core/Transform.h"

#include "tools/Common.h"
#include "tools/Logger.h"
#include "tools/Tools.h"

#include "bsdf/AlphaBSDF.h"
#include "bsdf/BumpMapping.h"
#include "bsdf/Conductor.h"
#include "bsdf/Diffuse.h"
#include "bsdf/Dielectric.h"
#include "bsdf/NormalMapping.h"
#include "bsdf/Phong.h"
#include "bsdf/TwoSides.h"
#include "texture/ConstantTexture.h"
#include "texture/ImageTexture.h"

#include <functional>
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
		indPos = 1;
		indN = 1;
		indUV = 1;

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
struct VertexObjHash {
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
					 const Parameters& params, 
					 std::vector<std::pair<int, BSDF::ptr>>* BSDFAndTriangleIndexTimes3)
{
	typedef std::unordered_map<VertexObj, uint32_t, VertexObjHash> VertexMap;

	bool useMtl = params.getBool("useMtl", false);

	Transform::ptr transform = params.getTransform("toWorld", std::shared_ptr<Transform>(new Transform));

	std::string filename = ::getGlobalFileSystem().resolve(params.getString("path", "")).string();

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

	int meshIndex = -1;
	bool nameIsGivenAfter = false;
	std::string materialName;

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
		else if (type == "g" && useMtl)
		{
			meshIndex++;
		}
		else if (type == "mtllib" && useMtl && myBSDFByName.size() == 0)
		{
			std::string mtlPath = tools::trim(line.substr(6, line.size() - 1));
			Filesystem tmp;
			std::filesystem::path f(filename);
			tmp.addSearchPath(f.parent_path());
			mtlPath = tmp.resolve(mtlPath).string();
			myBSDFByName = parseMtl(mtlPath, params);
		}
		else if (type == "usemtl" && useMtl && BSDFAndTriangleIndexTimes3)
		{
			//Assume usemtl is given after geometry and before faces
			//myNewMtlIndice = (int)vertices.size();

			//If name is given ater geometry
			if (indices.size() == 0) {
				nameIsGivenAfter = true;
				materialName = tools::trim(line.substr(6, line.size() - 1));
			}
			else
			{
				if (nameIsGivenAfter) {
					BSDF::ptr bsdf = myBSDFByName[materialName];
					int index = int(indices.size());
					BSDFAndTriangleIndexTimes3->push_back(std::make_pair(index, bsdf));
					materialName = tools::trim(line.substr(6, line.size() - 1));
				}
				else {
					materialName = tools::trim(line.substr(6, line.size() - 1));
					BSDF::ptr bsdf = myBSDFByName[materialName];
					int index = int(indices.size());
					BSDFAndTriangleIndexTimes3->push_back(std::make_pair(index, bsdf));
				}
			}
		
			//myBSDFByMeshIndex[]
			//meshIndex++;
			//myLastMtlIndice = myNewMtlIndice;
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

	if (nameIsGivenAfter) {
		BSDF::ptr bsdf = myBSDFByName[materialName];
		int index = int(indices.size());
		BSDFAndTriangleIndexTimes3->push_back(std::make_pair(index, bsdf));
	}

	return true;
}

std::map<std::string, BSDF::ptr> ObjLoader::parseMtl(const std::string& path, const Parameters& params)
{
	std::map<std::string, BSDF::ptr> res;

	std::ifstream is(path);
	if (is.fail())
	{
		ILogger::log(ILogger::ERRORS) << "Mtl file " << path << " cannot be open.\n";
		return res;
	}

	std::string currentName;
	std::string line;
	
	int illum = 0;
	real Ns = 0, Ni = 0;
	Texture::ptr d, Kd, Ks, bumpMap;

	bool bumpIsNormal = params.getBool("bumpIsNormal", false);

	while (std::getline(is, line))
	{
		line = tools::trim(line);
		std::istringstream sLine(line);

		std::string type;
		sLine >> type;

		if (type == "newmtl") {
			if (currentName != "")
			{
				addBSDF(res, currentName, illum, Ns, Ni, d, Kd, Ks, bumpMap, bumpIsNormal);
			}
			Ns = 0, Ni = 0;
			d = Kd = Ks = bumpMap = nullptr;
			currentName = tools::trim(line.substr(6, line.size() - 1));

		}
		else if (type == "Ns")
		{
			sLine >> Ns;
		}
		else if (type == "Ni")
		{
			sLine >> Ni;
		}
		else if (type == "d")
		{
			Color3 dCol;
			real g, b;
			sLine >> dCol.r();
			if (sLine >> g >> b) {
				dCol.g() = g; dCol.b() = b;
			}
			else {
				dCol.g() = dCol.r();
				dCol.b() = dCol.r();
			}
			d = Texture::ptr(new ConstantTexture(dCol));
		}
		else if (type == "map_d")
		{
			std::string path(tools::trim(line.substr(type.size(), line.size())));
			Parameters p;
			p.addString("path", ::getGlobalFileSystem().resolve(path).string());
			p.addReal("gamma", 1.);
			p.addString("channel", "a");
			p.addBool("invertY", true);

			d = Texture::ptr(new ImageTexture(p));
		}
		else if (type == "Tr")
		{
		}
		else if (type == "Tf")
		{
		}
		else if (type == "illum")
		{
			sLine >> illum;
		}
		else if (type == "Kd")
		{
			Color3 KdCol;
			sLine >> KdCol.r() >> KdCol.g() >> KdCol.b();
			Kd = Texture::ptr(new ConstantTexture(KdCol));
		}
		else if (type == "Ks")
		{
			Color3 KsCol;
			sLine >> KsCol.r() >> KsCol.g() >> KsCol.b();
			Ks = Texture::ptr(new ConstantTexture(KsCol));
		}
		else if (type == "map_Kd")
		{
			std::string path = tools::trim(line.substr(type.size(), line.size() - 1));
			Parameters p;
			p.addString("path", ::getGlobalFileSystem().resolve(path).string());
			p.addBool("invertY", true);
		
			Kd = Texture::ptr(new ImageTexture(p));
		}
		else if (type == "bump" || type == "map_Bump")
		{
			std::string bumpPath = tools::trim(line.substr(type.size(), line.size() - 1));
			Parameters p;
			p.addString("path", ::getGlobalFileSystem().resolve(bumpPath).string());
			p.addReal("gamma", 1.);
			p.addBool("invertY", true);

			bumpMap = Texture::ptr(new ImageTexture(p));		
		}
	}

	addBSDF(res, currentName, illum, Ns, Ni, d, Kd, Ks, bumpMap, bumpIsNormal);

	return res;
}

void ObjLoader::addBSDF(std::map<std::string, BSDF::ptr>& map, 
	const std::string& name, int illum, real Ns, real Ni, Texture::ptr d, 
	Texture::ptr Kd, Texture::ptr Ks, Texture::ptr bumpMap, bool bumpIsNormal)
{
	Parameters p;
	BSDF::ptr bsdf;

	if (illum == 2)
	{
		p.addTexture("kd", Kd);
		p.addTexture("ks", Ks);
		p.addReal("alpha", Ns);
		bsdf = BSDF::ptr(new Phong(p));
	}
	else if (illum == 4 || illum == 6 || illum == 7 || illum == 9)
	{
		bsdf = BSDF::ptr(new Dielectric(p));
	}
	else if (illum == 5 || illum == 8)
	{
		p.addString("material", "silver");
		bsdf = BSDF::ptr(new Conductor(p));
	}
	else
	{
		p.addTexture("albedo", Kd);
		bsdf = BSDF::ptr(new Diffuse(p));
	}

	if (bumpMap != nullptr)
	{
		Parameters bumpParams;
		bumpParams.addBSDF("bsdf", bsdf);
		
		if (bumpIsNormal) {
			bumpParams.addTexture("normalMap", bumpMap);
			BSDF::ptr bump(new NormalMapping(bumpParams));
			bsdf = bump;
		}
		else {
			bumpParams.addTexture("bumpMap", bumpMap);
			BSDF::ptr bump(new BumpMapping(bumpParams));
			bsdf = bump;
		}
	}

	if (d != nullptr && d->getAverage().average() < real(1.))
	{
		Parameters alphaParams;
		alphaParams.addBSDF("bsdf", bsdf);
		alphaParams.addTexture("alpha", d);

		BSDF::ptr alpha(new AlphaBSDF(alphaParams));
		bsdf = alpha;
	}

	if (illum != 4 && illum != 6 && illum != 7 && illum != 9)
	{
		Parameters twoSidesParams;
		twoSidesParams.addBSDF("bsdf", bsdf);

		BSDF::ptr twoSides(new TwoSides(twoSidesParams));
		bsdf = twoSides;
	}

	map[name] = bsdf;
}