#include "Triangle.h"

#include "Intersection.h"
#include "Mapping.h"
#include "Tools.h"

#include <Eigen/Dense>

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Triangle::Triangle(const Parameters & params, Mesh::ptr mesh, int indice)
:myMesh(mesh)
,myIndice(indice)
{
	myObjectToWorld = params.getTransform("toWorld", Transform::ptr(new Transform));
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Triangle::~Triangle(void)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Normal3d Triangle::normal(const Point3d&) const
{
	const Point3d& p0 =  myMesh->myVertices[myMesh->myIndices[myIndice]];
	const Point3d& p1 = myMesh->myVertices[myMesh->myIndices[myIndice + 1]];
	const Point3d& p2 = myMesh->myVertices[myMesh->myIndices[myIndice + 2]];
	Vector3d e1 = p1 - p0;
	Vector3d e2 = p2 - p0;

#ifdef USE_ALIGN
	Normal3d normal = e1.cross3(e2);
#else
	Normal3d normal = e1.cross(e2);
#endif
	normal.normalize();
	return normal;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
//bool Triangle::intersection(const Ray& ray, real& t, DifferentialGeometry& trueGeom, DifferentialGeometry& shadingGeom, bool shadowRay)
bool Triangle::intersection(const Ray& ray, real& t, Point2d& uv)
{
	const Point3d& p0 = myMesh->myVertices[myMesh->myIndices[myIndice]];
	const Point3d& p1 = myMesh->myVertices[myMesh->myIndices[myIndice + 1]];
	const Point3d& p2 = myMesh->myVertices[myMesh->myIndices[myIndice + 2]];
	Vector3d e1 = p1 - p0;
	Vector3d e2 = p2 - p0;
	Vector3d s = ray.myOrigin - p0;

#ifdef USE_ALIGN
	Vector3d s1 = ray.direction().cross3(e2);
	Vector3d s2 = s.cross3(e1);
#else
	Vector3d s1 = ray.direction().cross(e2);
	Vector3d s2 = s.cross(e1);
#endif

	real d = s1.dot(e1);
	
	//If det ~ 0, rays is in the plane of triangle
	if (std::abs(d) < 1e-8)
		return false;
	real inv = 1 / d;

	//Point2d uv;

	//b1
	uv.x() = inv * s1.dot(s);
	if (uv.x() < 0. || uv.x() > 1.)
		return false;

	//b2
	uv.y() = inv * s2.dot(ray.direction());

	if (uv.y() < 0. || uv.x() + uv.y() > 1.)
		return false;
	
	t  = inv * s2.dot(e2);

	if (t < ray.myMinT || t > ray.myMaxT)
		return false;

	return true;

	//if (shadowRay)
	//	return true;

	////Point2d uv(b1, b2);
	//Vector3d bary;
	//bary << 1 - uv.sum(), uv;
	///* Compute the intersection positon accurately
	//using barycentric coordinates */
	////Point3d p = bary.x() * p0 + bary.y() * p1 + bary.z() * p2;

	///* Compute proper texture coordinates if provided by the mesh */
	////if (myMesh->myUVs.size() > 0)
	////	trueGeom.uv = bary.x() * UV.col(idx0) +
	////	bary.y() * UV.col(idx1) +
	////	bary.z() * UV.col(idx2);

	////Compute the geometric normal
	//trueGeom = DifferentialGeometry((p1 - p0).cross(p2 - p0).normalized());

	//if (myMesh->myNormals.size() > 0) {
	//	/* Compute the shading frame. Note that for simplicity,
	//	the current implementation doesn't attempt to provide
	//	tangents that are continuous across the surface. That
	//	means that this code will need to be modified to be able
	//	use anisotropic BRDFs, which need tangent continuity */

	//	Normal3d tmpN = bary.x() * myMesh->myNormals[myMesh->myIndices[myIndice]] +
	//		bary.y() * myMesh->myNormals[myMesh->myIndices[myIndice + 1]] +
	//		bary.z() * myMesh->myNormals[myMesh->myIndices[myIndice + 2]];
	//	shadingGeom = DifferentialGeometry(tmpN.normalized());
	//}
	//else {
	//	shadingGeom = trueGeom;
	//}
	//
	//return true;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void Triangle::getDifferentialGeometry(DifferentialGeometry& trueGeometry,
	DifferentialGeometry& shadingGeometry, Intersection& inter)
{
	int index0 = myMesh->myIndices[myIndice];
	int index1 = myMesh->myIndices[myIndice + 1];
	int index2 = myMesh->myIndices[myIndice + 2];

	const Point3d& p0 = myMesh->myVertices[index0];
	const Point3d& p1 = myMesh->myVertices[index1];
	const Point3d& p2 = myMesh->myVertices[index2];

	Vector3d bary;
#ifdef USE_ALIGN
	bary << 1 - inter.myUv.sum(), inter.myUv, 0;
#else
	bary << 1 - inter.myUv.sum(), inter.myUv;
#endif

	/* Compute the intersection positon accurately
	using barycentric coordinates */
	inter.myPoint = bary.x() * p0 + bary.y() * p1 + bary.z() * p2;

	/* Compute proper texture coordinates if provided by the mesh */
	if (myMesh->myUVs.size() >= 3)//4)
	{
		const Point2d& uv0 = myMesh->myUVs[index0];
		const Point2d& uv1 = myMesh->myUVs[index1];
		const Point2d& uv2 = myMesh->myUVs[index2];

		inter.myUv = bary.x() * uv0 + bary.y() * uv1 + bary.z() * uv2;
	}

	//Compute the geometric normal
#ifdef USE_ALIGN
	trueGeometry = DifferentialGeometry((p1 - p0).cross3(p2 - p0).normalized());
#else
	trueGeometry = DifferentialGeometry((p1 - p0).cross(p2 - p0).normalized());
#endif

	if (myMesh->myNormals.size() > 0) {
		/* Compute the shading frame. Note that for simplicity,
		the current implementation doesn't attempt to provide
		tangents that are continuous across the surface. That
		means that this code will need to be modified to be able
		use anisotropic BRDFs, which need tangent continuity */

		Normal3d tmpN = bary.x() * myMesh->myNormals[index0] +
			bary.y() * myMesh->myNormals[index1] +
			bary.z() * myMesh->myNormals[index2];
		shadingGeometry = DifferentialGeometry(tmpN.normalized());
	}
	else {
		shadingGeometry = trueGeometry;
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void Triangle::sample(const Point2d& p, Point3d& sampled, Normal3d& normal)
{
	int index0 = myMesh->myIndices[myIndice];
	int index1 = myMesh->myIndices[myIndice + 1];
	int index2 = myMesh->myIndices[myIndice + 2];

	const Point3d& p0 = myMesh->myVertices[index0];
	const Point3d& p1 = myMesh->myVertices[index1];
	const Point3d& p2 = myMesh->myVertices[index2];

	Point3d baryCoords = Mapping::squareToTriangle(p);

	sampled = p0 * baryCoords.x() + p1 * baryCoords.y() + p2 * baryCoords.z();

	if (myMesh->myNormals.size() > 0) {
		normal = baryCoords.x() * myMesh->myNormals[index0] +
			baryCoords.y() * myMesh->myNormals[index1] +
			baryCoords.z() * myMesh->myNormals[index2];
		normal.normalize();
	}
	else
	{
		#ifdef USE_ALIGN
			normal = (p1 - p0).cross3(p2 - p0).normalized();
		#else
			normal = (p1 - p0).cross(p2 - p0).normalized();
		#endif

		normal.normalize();
	}
}

void Triangle::sample(const Point2d& p, Point3d& sampled, Normal3d& normal, const Point3d& p0, const Point3d& p1, const Point3d& p2, Normal3d* n0, Normal3d* n1, Normal3d* n2)
{
	Point3d baryCoords = Mapping::squareToTriangle(p);

	sampled = p0 * baryCoords.x() + p1 * baryCoords.y() + p2 * baryCoords.z();

	if (n0 != nullptr && n1 != nullptr && n2 != nullptr) {
		normal = baryCoords.x() * *n0 +
			baryCoords.y() * *n1 +
			baryCoords.z() * *n2;
		normal.normalize();
	}
	else
	{
#ifdef USE_ALIGN
		normal = (p1 - p0).cross3(p2 - p0).normalized();
#else
		normal = (p1 - p0).cross(p2 - p0).normalized();
#endif

		normal.normalize();
	}
}

//real Triangle::pdf(const Point3d& pFrom, Point3d& sampled, Normal3d& normal)
//{
//	Vector3d lightToShape = pFrom - sampled;
//	real distance = lightToShape.norm();
//	lightToShape.normalize();
//
//	real cosine = normal.dot(lightToShape);
//	if (cosine >= 0)
//	{
//		//Convert area pdf to pdf wrt solid angle (solid angle subtented by the surface)
//		//dw = cos theta * dA / r2
//		//and pdf = 1 / dw
//		//   _____
//		//	|____ | <- surface
//		//	\     /
//		//	 \   /
//		//	  \ /
//		return distance * distance / (surfaceArea() * cosine);
//	}
//	else
//	{
//		return 0.;
//	}
//
//	return 0.;
//}





//const Point3d p0 = myVertexIndices[0], p1 = myVertexIndices[1], p2 = myVertexIndices[2];
//
///* Find vectors for two edges sharing v[0] */
//Vector3d edge1 = p1 - p0, edge2 = p2 - p0;
//
///* Begin calculating determinant - also used to calculate U parameter */
//Vector3d pvec = ray.myDirection.cross(edge2);
//
///* If determinant is near zero, ray lies in plane of triangle */
//float det = edge1.dot(pvec);
//
//if (det > -1e-8f && det < 1e-8f)
//	return tools::MAX_REAL;
//float inv_det = 1.0f / det;
//
///* Calculate distance from v[0] to ray origin */
//Vector3d tvec = ray.myOrigin - p0;
//
///* Calculate U parameter and test bounds */
//real u = tvec.dot(pvec) * inv_det;
//if (u < 0.0 || u > 1.0)
//return tools::MAX_REAL;
//
///* Prepare to test V parameter */
//Vector3d qvec = tvec.cross(edge1);
//
///* Calculate V parameter and test bounds */
//real v = ray.myDirection.dot(qvec) * inv_det;
//if (v < 0.0 || u + v > 1.0)
//return tools::MAX_REAL;
//
///* Ray intersects triangle -> compute t */
//real tee = edge2.dot(qvec) * inv_det;
//
//return tee;