#pragma once
#include "GeometricShape.h"
#include "Geometry.h"

#include "Mesh.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief Triangle geometric shape.
///////////////////////////////////////////////////////////////////////////////
class Triangle :
	public GeometricShape
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

public:
	typedef std::shared_ptr<Triangle> ptr;

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor.
	/// 
	/// \param params Some specific parameters (Transform : toWorld).
	/// \param mesh The parent mesh.
	/// \param indice The position of this triangle in the mesh
	///////////////////////////////////////////////////////////////////////////
	Triangle(const Parameters& params, Mesh::ptr mesh, int indice);
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Destructor.
	///////////////////////////////////////////////////////////////////////////
	~Triangle(void);

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Computes the normal vector at the given point
	///////////////////////////////////////////////////////////////////////////
	Normal3d normal(const Point3d& intersection) const override;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Determines if the given ray intersects the shape.
	///
	/// \param ray The considered ray.
	///
	/// \param t Out parameter indicating the parametric distance of the 
	/// intersection point to the ray origin, if an intersection was found.
	///
	/// \param shadowRay Indicates whether the ray is a shadow ray (faster 
	/// intersection routines) of not.
	///////////////////////////////////////////////////////////////////////////
	//bool intersection(const Ray& ray, real& t, DifferentialGeometry& trueGeom, DifferentialGeometry& shadingGeom, bool shadowRay = false) override;
	bool intersection(const Ray& ray, real& t, Point2d& uv) override;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Computes the triangle local bounding box.
	///////////////////////////////////////////////////////////////////////////
	virtual BoundingBox getLocalBoundingBox() const override
	{
		return myObjectToWorld->inv().transform(getWorldBoundingBox()); 
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Computes the world bounding box
	///////////////////////////////////////////////////////////////////////////
	BoundingBox getWorldBoundingBox() const override
	{
		Point3d p[3];
		p[0] = myMesh->myVertices[myMesh->myIndices[myIndice]];
		p[1] = myMesh->myVertices[myMesh->myIndices[myIndice + 1]];
		p[2] = myMesh->myVertices[myMesh->myIndices[myIndice + 2]];

		real xMin = p[0].x(), yMin = p[0].y(), zMin = p[0].z();
		real xMax = p[0].x(), yMax = p[0].y(), zMax = p[0].z();

		for (auto i = 1; i < 3; i++)
		{
			xMin = std::min(p[i].x(), xMin);
			yMin = std::min(p[i].y(), yMin);
			zMin = std::min(p[i].z(), zMin);

			xMax = std::max(p[i].x(), xMax);
			yMax = std::max(p[i].y(), yMax);
			zMax = std::max(p[i].z(), zMax);
		}

		return BoundingBox(Point3d(xMin, yMin, zMin), Point3d(xMax, yMax, zMax));
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Computes the triangle centroid, aka barycenter.
	///////////////////////////////////////////////////////////////////////////
	Point3d getCentroid() override
	{
		Point3d p[3];
		p[0] = myMesh->myVertices[myMesh->myIndices[myIndice]];
		p[1] = myMesh->myVertices[myMesh->myIndices[myIndice + 1]];
		p[2] = myMesh->myVertices[myMesh->myIndices[myIndice + 2]];
		return (1.f / 3) * (p[0] + p[1] + p[2]);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Computes the differential geometry at the intersection point.
	///////////////////////////////////////////////////////////////////////////
	void getDifferentialGeometry(DifferentialGeometry& trueGeometry,
		DifferentialGeometry& shadingGeometry, Intersection& inter) override;

	void sample(const Point2d& p, Point3d& sampled, Normal3d& normal) override;

	static void sample(const Point2d& p, Point3d& sampled, Normal3d& normal, const Point3d& p0, const Point3d& p1, const Point3d& p2, Normal3d* n0, Normal3d* n1, Normal3d* n2);
	//real pdf(const Point3d& p, Point3d& sampled, Normal3d& normal) override;

	real surfaceArea() override
	{
		int index0 = myMesh->myIndices[myIndice];
		int index1 = myMesh->myIndices[myIndice + 1];
		int index2 = myMesh->myIndices[myIndice + 2];

		const Point3d& p0 = myMesh->myVertices[index0];
		const Point3d& p1 = myMesh->myVertices[index1];
		const Point3d& p2 = myMesh->myVertices[index2];
#ifdef USE_ALIGN
		return 0.5f * Vector3d((p1 - p0).cross3(p2 - p0)).norm();
#else
		return 0.5f * Vector3d((p1 - p0).cross(p2 - p0)).norm();
#endif
	}

	static inline real triSurfaceArea(const Point3d& p0, const Point3d& p1, const Point3d& p2)
	{
#ifdef USE_ALIGN
		return 0.5f * Vector3d((p1 - p0).cross3(p2 - p0)).norm();
#else
		return 0.5f * Vector3d((p1 - p0).cross(p2 - p0)).norm();
#endif
	}
		
public:
	Mesh::ptr myMesh;
	int myIndice;
};

