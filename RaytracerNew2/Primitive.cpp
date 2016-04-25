#include "Primitive.h"

#include "Intersection.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
IPrimitive::IPrimitive()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
IPrimitive::~IPrimitive()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
bool SimplePrimitive::intersection(const Ray & ray, Intersection & inter, bool shadowRay)
{
	double t;
	//DifferentialGeometry trueGeom;
	//DifferentialGeometry shadingGeom;
	Point2d uv;
	//if (myObject->intersection(ray, t, trueGeom, shadingGeom, shadowRay))
	if (myObject->intersection(ray, t, uv))
	{
		inter.myPoint = ray.myOrigin + t * ray.direction();
		inter.t = t;
		inter.myUv = uv;
		//inter.myTrueGeometry = trueGeom;
		//inter.myShadingGeometry = shadingGeom;
		inter.myPrimitive = shared_from_this();
			
		return true;
	}
	
	return false;
}


//BoundingBox SimplePrimitive::getWorldBoundingBox()
//{
//	return myObject->getWorldBoundingBox();
//}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
bool InstancePrimitive::intersection(const Ray & ray, Intersection & inter, bool shadowRay)
{
	Ray transformedRay = myWorldToObjectWorldTransform->transform(ray);
	return myPrimitive->intersection(transformedRay, inter, shadowRay);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
BoundingBox InstancePrimitive::getWorldBoundingBox()
{
	return myPrimitive->getWorldBoundingBox();
}
