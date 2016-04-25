#include "Mesh.h"

#include "MeshFactory.h"
#include "Primitive.h"
#include "ObjectFactoryManager.h"
#include "Triangle.h"

Mesh::Mesh(const std::string& name,
	const std::vector<Point3d, Eigen::aligned_allocator<Point3d>>& vert,
	const std::vector<Normal3d, Eigen::aligned_allocator<Normal3d>>& normals,
	const std::vector<Point2d, Eigen::aligned_allocator<Point2d>>& uvs,
	const std::vector<int>& indices)
:myVertices(vert)
,myNormals(normals)
,myUVs(uvs)
,myIndices(indices)
,mySurfaceArea(0.)
{
}

Mesh::~Mesh()
{
}

void Mesh::sample(const Point2d& p, Point3d& sampled, Normal3d& normal)
{
	Point2d sample(p);

	//choose a triangle wrt the surface
	int index = rng.random(0, myIndices.size() / 3 - 1);
	//int index = myCDF.sampleAndReuse(sample.x());
	myLightTriangles[index]->sample(sample, sampled, normal);
}

double Mesh::pdf(const Point3d& pFrom, const Point3d& sampled, const Normal3d& normal)
{
	Vector3d lightToShape = pFrom - sampled;
	double distance = lightToShape.norm();
	lightToShape.normalize();

	double cosine = normal.dot(lightToShape);
	if (cosine > 0)
	{
		//Convert area pdf to pdf wrt solid angle (solid angle subtented by the surface)
		//dw = cos theta * dA / r2
		//and pdf = 1 / dw
		//   _____
		//	|____ | <- surface
		//	\     /
		//	 \   /
		//	  \ /
		return distance * distance / (surfaceArea() * cosine);
	}
	else
	{
		return 0.;
	}		
}

void Mesh::addLightTriangle(std::shared_ptr<GeometricShape> triangle)
{
	double surface = triangle->surfaceArea();
	myCDF.add(surface);
	myLightTriangles.push_back(triangle);
	mySurfaceArea += surface;
}

RT_REGISTER_TYPE_WITH_FACTORY(Mesh, IPrimitive, MeshFactory)