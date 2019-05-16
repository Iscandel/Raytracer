#include "GeometricShape.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
GeometricShape::GeometricShape(const Parameters& params)
{
	myObjectToWorld = params.getTransform("toWorld", Transform::ptr(new Transform));
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
GeometricShape::~GeometricShape(void)
{
}
