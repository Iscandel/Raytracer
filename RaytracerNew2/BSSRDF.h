#pragma once

#include "Color.h"
#include "RaytracerObject.h"
#include "WithSmartPtr.h"

#include <memory>

class ISampledShape;
class Intersection;

class BSSRDF : public RaytracerObject, public WithSmartPtr<BSSRDF>
{
public:
	BSSRDF();
	~BSSRDF();
	
	void setShape(std::shared_ptr<ISampledShape> shape) { myShape = shape; }

	virtual Color eval(const Point3d& pos, const Intersection& inter, const Vector3d& dir) = 0;

protected:
	std::weak_ptr<ISampledShape> myShape;
};

