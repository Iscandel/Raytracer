#include "Mesh.h"

#include "factory/MeshFactory.h"
#include "core/Primitive.h"
#include "factory/ObjectFactoryManager.h"
#include "geometricShape/Triangle.h"

Mesh::Mesh(const std::string&,
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
	for (int i = 0; i < indices.size(); i += 3)
	{
		int index0 = myIndices[i];
		int index1 = myIndices[i + 1];
		int index2 = myIndices[i + 2];

		const Point3d& p0 = myVertices[index0];
		const Point3d& p1 = myVertices[index1];
		const Point3d& p2 = myVertices[index2];
		real surface = Triangle::triSurfaceArea(p0, p1, p2);
		myCDF.add(surface);
		mySurfaceArea += surface;
	}	
}

Mesh::~Mesh()
{
}

void Mesh::sample(const Point2d& p, Point3d& sampled, Normal3d& normal)
{
	Point2d sample(p);

	//choose a triangle wrt the surface
	//int index = rng.random(0, myIndices.size() / 3 - 1);
	int index = myCDF.sampleAndReuse(sample.x());

	int index0 = myIndices[index * 3];
	int index1 = myIndices[index * 3 + 1];
	int index2 = myIndices[index * 3 + 2];

	const Point3d& p0 = myVertices[index0];
	const Point3d& p1 = myVertices[index1];
	const Point3d& p2 = myVertices[index2];

	Normal3d* n0 = nullptr;
	Normal3d* n1 = nullptr;
	Normal3d* n2 = nullptr; 
	if (myNormals.size() != 0)
	{
		n0 = &myNormals[index0];
		n1 = &myNormals[index1];
		n2 = &myNormals[index2];
	}

	Triangle::sample(sample, sampled, normal, p0, p1, p2, n0, n1, n2);
	//Triangle triangle(Parameters(), shared_from_this(), index * 3);
	//triangle.sample(sample, sampled, normal);
	//myLightTriangles[index]->sample(sample, sampled, normal);
}

real Mesh::pdf(const Point3d& pFrom, const Point3d& sampled, const Normal3d& normal)
{
	Vector3d lightToShape = pFrom - sampled;
	real distance = lightToShape.norm();
	lightToShape.normalize();

	real cosine = normal.dot(lightToShape);
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
	//real surface = triangle->surfaceArea();
	//myCDF.add(surface);
	myLightTriangles.push_back(triangle);
	//mySurfaceArea += surface;
}

RT_REGISTER_TYPE_WITH_FACTORY(Mesh, IPrimitive, MeshFactory)