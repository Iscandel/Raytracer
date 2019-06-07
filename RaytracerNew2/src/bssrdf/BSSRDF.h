#pragma once

#include "core/Color.h"
#include "core/RaytracerObject.h"
#include "tools/WithSmartPtr.h"

#include <memory>

class ISampledShape;
class Intersection;

class BSSRDF : public RaytracerObject, public WithSmartPtr<BSSRDF>
{
public:
	BSSRDF();
	virtual ~BSSRDF();
	
	void setShape(std::shared_ptr<ISampledShape> shape) { myShape = shape; }

	virtual Color eval(const Point3d& pos, const Intersection& inter, const Vector3d& dir) = 0;

protected:
	std::weak_ptr<ISampledShape> myShape;
};

