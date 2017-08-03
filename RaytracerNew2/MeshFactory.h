#pragma once
#include "ObjectFactory.h"

#include "Bvh.h"
#include "ObjLoader.h"
#include "Logger.h"
#include "Timer.h"
#include "Triangle.h"

#include "AreaLight.h"

#include <Eigen/StdVector>

///////////////////////////////////////////////////////////////////////////
/// \brief Dedicated factory for meshes. Meshes deal with a triangle list 
/// that have to be instanced here, with their associated BSDF.
///////////////////////////////////////////////////////////////////////////
template<class BaseType, class ObjectType, typename ...Args>
class MeshFactory : public ObjectFactory<BaseType>
{
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor.
	///////////////////////////////////////////////////////////////////////////
	MeshFactory()
		:ObjectFactory<BaseType>(ObjectStaticType<ObjectType>::get())
	{
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Creates a new mesh.
	///
	/// \param params : A list of custom parameters.
	///
	/// \return A smart pointer to the newly created primitive containing the mesh.
	///////////////////////////////////////////////////////////////////////////
	//typename BaseType::ptr create(Args&&... args) override//const std::string& name) 
	typename BaseType::ptr create(const Parameters& params) override
	{
		Timer time;

		std::vector<Point3d, Eigen::aligned_allocator<Point3d>> vertices;
		std::vector<Normal3d, Eigen::aligned_allocator<Normal3d>> normals;
		std::vector<Point2d, Eigen::aligned_allocator<Point2d>> UVs;
		std::vector<int> indices;

		std::vector<IPrimitive::ptr> triangles;

		//const IMeshLoader& loader = IMeshLoader::getLoader(params);
		ObjLoader loader;
		if (!loader.read(vertices, normals, UVs, indices, params))
		{
			ILogger::log(ILogger::ERRORS) << "Unexpected error while loading mesh file.\n";
			return nullptr;
		}

		//Initialize the mesh with loaded geometric data
		Mesh::ptr mesh = std::shared_ptr<Mesh>(new Mesh("", vertices, normals, UVs, indices));
		
		BSDF::ptr bsdf = params.getBSDF("bsdf", BSDF::ptr());
		Light::ptr light = params.getLight("light", Light::ptr());
		Medium::ptr interiorMedium = params.getMedium("interior", Medium::ptr());
		Medium::ptr exteriorMedium = params.getMedium("exterior", Medium::ptr());

		if(light)
			((AreaLight*) light.get())->setShape(mesh);

		//Create the triangles and associate them a BSDF defined in the params (if any)
		for (unsigned int i = 0; i < indices.size(); i += 3)
		{
			GeometricShape::ptr shape = std::shared_ptr<Triangle>(new Triangle(params, mesh, i));
			
			IPrimitive::ptr triPrimitive(std::make_shared<SimplePrimitive>(shape, bsdf, interiorMedium, exteriorMedium));
			if (light)
			{
				mesh->addLightTriangle(shape);
				triPrimitive->addLight(light);
			}
			triangles.push_back(triPrimitive);
		}

		//Initialize CDF function
		mesh->initialize();

		ILogger::log(ILogger::ALL) << "Object created in " << time.elapsedTime() << "s ("
			<< vertices.size() << " vertices, " << indices.size() / 3 << " triangles).\n";

		//Store the triangles in a suitable acceleration structure
		typename BaseType::ptr primitive = std::make_shared<Bvh>(triangles);
		if (light)
		{
			primitive->addLight(light);
		}

		return primitive;
	}

};

