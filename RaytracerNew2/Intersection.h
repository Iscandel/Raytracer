#pragma once

#include "DifferentialGeometry.h"
#include "Primitive.h"
#include "Geometry.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief Data structure that stores intersection informations.
///////////////////////////////////////////////////////////////////////////////
class Intersection
{
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor.
	///////////////////////////////////////////////////////////////////////////
	Intersection();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Destructor.
	///////////////////////////////////////////////////////////////////////////
	~Intersection();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Transforms the given vector in a local coordinates vector.
	///////////////////////////////////////////////////////////////////////////
	Vector3d toLocal(const Vector3d& v)
	{
		return myShadingGeometry.toLocal(v);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Transforms the given local vector in a world coordinates vector.
	///////////////////////////////////////////////////////////////////////////
	Vector3d toWorld(const Vector3d& v)
	{
		return myShadingGeometry.toWorld(v);
	}

	///Parametric intersection distance
	double t;
	///True differential geometry (for ex. face normal and not interpolated
	///normal at the intersection point)
	DifferentialGeometry myTrueGeometry;
	///Geometry at the intersection point
	DifferentialGeometry myShadingGeometry;
	///intersected Primitive
	IPrimitive::ptr myPrimitive;
	///UV coordinates
	Point2d myUv;
	///Intersection point
	Point3d myPoint;

	bool computeIntersect;
};
