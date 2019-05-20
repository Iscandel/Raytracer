#include "Cube.h"

#include "factory/ObjectFactoryManager.h"

#include "core/Primitive.h"
#include "factory/MeshFromShapeFactory.h"

Cube::Cube(const Parameters& params)
{
	myTransform = params.getTransform("toWorld", std::shared_ptr<Transform>(new Transform));
}


Cube::~Cube()
{
}

void Cube::computeMeshObject(std::vector<Point3d, Eigen::aligned_allocator<Point3d>>& vertices,
	std::vector<Normal3d, Eigen::aligned_allocator<Normal3d>>& normals,
	std::vector<Point2d, Eigen::aligned_allocator<Point2d>>& UVs,
	std::vector<int>& indices)
{
	vertices = std::vector<Point3d, Eigen::aligned_allocator<Point3d>>
	{ { 1, -1, -1 },{ 1, -1, 1 },{ -1, -1, 1 },{ -1, -1, -1 },{ 1, 1, -1 },{ -1, 1, -1 },{ -1, 1, 1 },{ 1, 1, 1 },{ 1, -1, -1 },{ 1, 1, -1 },{ 1, 1, 1 },{ 1, -1, 1 },{ 1, -1, 1 },{ 1, 1, 1 },{ -1, 1, 1 },{ -1, -1, 1 },{ -1, -1, 1 },{ -1, 1, 1 },{ -1, 1, -1 },{ -1, -1, -1 },{ 1, 1, -1 },{ 1, -1, -1 },{ -1, -1, -1 },{ -1, 1, -1 } };

	normals = std::vector<Normal3d, Eigen::aligned_allocator<Normal3d>>
	{ { 0, -1, 0 },{ 0, -1, 0 },{ 0, -1, 0 },{ 0, -1, 0 },{ 0, 1, 0 },{ 0, 1, 0 },{ 0, 1, 0 },{ 0, 1, 0 },{ 1, 0, 0 },{ 1, 0, 0 },{ 1, 0, 0 },{ 1, 0, 0 },{ 0, 0, 1 },{ 0, 0, 1 },{ 0, 0, 1 },{ 0, 0, 1 },{ -1, 0, 0 },{ -1, 0, 0 },{ -1, 0, 0 },{ -1, 0, 0 },{ 0, 0, -1 },{ 0, 0, -1 },{ 0, 0, -1 },{ 0, 0, -1 } };

	UVs = std::vector<Point2d, Eigen::aligned_allocator<Point2d>>
	{ { 0, 1 },{ 1, 1 },{ 1, 0 },{ 0, 0 },{ 0, 1 },{ 1, 1 },{ 1, 0 },{ 0, 0 },{ 0, 1 },{ 1, 1 },{ 1, 0 },{ 0, 0 },{ 0, 1 },{ 1, 1 },{ 1, 0 },{ 0, 0 },{ 0, 1 },{ 1, 1 },{ 1, 0 },{ 0, 0 },{ 0, 1 },{ 1, 1 },{ 1, 0 },{ 0, 0 } };

	indices = std::vector<int>{ 0,1,2, 3,0,2, 4,5,6, 7,4,6, 8,9,10, 11,8,10, 12,13,14, 15,12,14, 16,17,18, 19,16,18, 20,21,22, 23,20,22 };

	for (Point3d& p : vertices)
		p = myTransform->transform(p);

	for (Normal3d& n : normals)
		n = myTransform->transform(n);


	////vertices = std::vector<Point3d, Eigen::aligned_allocator<Point3d>>
	////{ {-1, -1, -1}, {1,-1,-1}, {1, 1, -1}, {-1, 1, -1,}, {-1, 1, 1}, {-1, -1, 1}, {1, 1, 1}, {1, -1, 1} };

	//vertices = std::vector<Point3d, Eigen::aligned_allocator<Point3d>>
	//{ { -1, -1, -1 },{ 1,-1,-1 },{ 1, -1, 1 },{ -1, -1, 1, },{ -1, 1, 1 },{ -1, 1, -1 },{ 1, 1, 1 },{ 1, 1, -1 } };

	//normals = std::vector<Normal3d, Eigen::aligned_allocator<Normal3d>>
	//{ {0, 0, -1}, {1, 0, 0}, {0, 0, 1}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0} };

	//for(Point3d& p : vertices)
	//	p = myTransform->transform(p);
	//
	////For hard corners, define different normal for the same vertices belonging to different faces
	////normals = std::vector<Normal3d, Eigen::aligned_allocator<Normal3d>>
	////{ {-0.5, -0.5, -0.5},{ 0.5, -0.5, -0.5 },{ 0.5, -0.5, 0.5 },{ -0.5, -0.5, 0.5 },{ -0.5, 0.5, 0.5 },{ -0.5, 0.5, -0.5 },{ 0.5, 0.5, 0.5 },{ 0.5, 0.5, -0.5 } };

	//for (Normal3d& n : normals)
	//	n = myTransform->transform(n);

	////UVs = std::vector<Point2d, Eigen::aligned_allocator<Point2d>>{};

	//indices = std::vector<int>{ 1,2,4, 2,3,4, 1,4,6, 4,5,6, 5,8,6, 5,7,8, 2,8,3, 3,8,7, 4,3,5, 5,3,7, 2,6,8, 1,6,2 };
	//
	//for (int i = 0; i < indices.size(); i++)
	//	indices[i]--;
}

RT_REGISTER_TYPE_WITH_FACTORY(Cube, IPrimitive, MeshFromShapeFactory)