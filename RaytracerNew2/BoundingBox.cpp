#include "BoundingBox.h"

#include "Geometry.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
BoundingBox::BoundingBox()
{
	myMin = Point3d(math::MAX_REAL, math::MAX_REAL, math::MAX_REAL);
	myMax = Point3d(-math::MAX_REAL, -math::MAX_REAL, -math::MAX_REAL);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
BoundingBox::BoundingBox(const Point3d& point) 
:myMin(point), myMax(point) 
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
BoundingBox::BoundingBox(const Point3d& p1, const Point3d& p2)
{
	myMin = Point3d(std::min(p1.x(), p2.x()), std::min(p1.y(), p2.y()), std::min(p1.z(), p2.z()));
	myMax = Point3d(std::max(p1.x(), p2.x()), std::max(p1.y(), p2.y()), std::max(p1.z(), p2.z()));
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
BoundingBox::~BoundingBox()
{
}
