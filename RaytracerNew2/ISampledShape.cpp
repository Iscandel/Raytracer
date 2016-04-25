#include "ISampledShape.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
ISampledShape::ISampledShape()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
ISampledShape::~ISampledShape()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double ISampledShape::pdf(const Point3d& pFrom, const Point3d& sampled, const Normal3d& normal)
{
	Vector3d lightToShape = pFrom - sampled;
	double distance = lightToShape.norm();
	lightToShape.normalize();

	double cosine = normal.dot(lightToShape);
	if (cosine >= 0)
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