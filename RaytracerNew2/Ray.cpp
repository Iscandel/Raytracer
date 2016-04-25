#include "Ray.h"

#include "Tools.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Ray::Ray()
:myMinT(tools::EPSILON)
,myMaxT(tools::MAX_DOUBLE)
{

}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Ray::Ray(const Point3d& origin, const Vector3d& dir)
//:myDirection(direction)
:myOrigin(origin)
,myMinT(tools::EPSILON)
,myMaxT(tools::MAX_DOUBLE)
{
	direction(dir);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Ray::Ray(const Point3d & origin, const Vector3d & dir, double minT, double maxT)
//:myDirection(direction)
:myOrigin(origin)
,myMinT(minT)
,myMaxT(maxT)
{
	direction(dir);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Ray::~Ray(void)
{
}
