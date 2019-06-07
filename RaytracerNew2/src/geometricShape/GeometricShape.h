#ifndef H__OBJECT_170520151951__H
#define H__OBJECT_170520151951__H

#include "core/DifferentialGeometry.h"
#include "core/Geometry.h"
#include "core/Ray.h"
#include "core/BoundingBox.h"
#include "core/Transform.h"
#include "core/Parameters.h"
#include "geometricShape/ISampledShape.h"
#include "core/RaytracerObject.h"

class Intersection;

#include <memory>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
/// \brief Superclass of shape. Provides abstract functions to compute geometric
/// quantities related to the shape.
///////////////////////////////////////////////////////////////////////////////
class GeometricShape : public ISampledShape, public RaytracerObject
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

public:
	typedef std::shared_ptr<GeometricShape> ptr;

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor.
	///////////////////////////////////////////////////////////////////////////
	GeometricShape() {}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor.
	///////////////////////////////////////////////////////////////////////////
	GeometricShape(const Parameters& params);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Destructor
	///////////////////////////////////////////////////////////////////////////
	virtual ~GeometricShape(void);

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Computes the centroid of the shape
	///
	/// \return The position of the centroid in world coordinates
	///////////////////////////////////////////////////////////////////////////
	virtual Point3d getCentroid() { throw std::runtime_error("[GeometricShape] centroid not implemented (not redefined in child ?)"); }

	///////////////////////////////////////////////////////////////////////////
	/// \brief Computes the normal vector at the given point
	///
	/// \param intersection Position of the considered point.
	///
	/// \return The normal vector
	///////////////////////////////////////////////////////////////////////////
	virtual Normal3d normal(const Point3d& intersection) const = 0;

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
	//virtual bool intersection(const Ray& ray, real& t, DifferentialGeometry& trueGeom, DifferentialGeometry& shadingGeom, bool shadowRay = false) = 0;
	virtual bool intersection(const Ray& ray,
		real& t,
		Point2d& uv) = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns the bounding box of the shape in local coordinates.
	///////////////////////////////////////////////////////////////////////////
	virtual BoundingBox getLocalBoundingBox() const = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Returns the bounding box of the shape in world coordinates.
	///////////////////////////////////////////////////////////////////////////
	virtual BoundingBox getWorldBoundingBox() const { return myObjectToWorld->transform(getLocalBoundingBox()); }

	virtual void getDifferentialGeometry(DifferentialGeometry& trueGeometry,
		DifferentialGeometry& shadingGeometry, Intersection& inter) = 0;

protected:
	Transform::ptr myObjectToWorld; 

	//static std::string myObjectType;
};




#endif
