#pragma once
#include "geometricShape/GeometricShape.h"
#include "core/Geometry.h"

#include <string>
#include <memory>

class Scene;

class System
{
public:
	typedef std::shared_ptr<System> ptr;
public:
	System(const std::string& name);
	~System(void);

	virtual Color compute(Scene& scene, const GeometricShape& obj, const Point3d& inter, const Ray& ray, int level) = 0;

	const std::string& getName() const {return myName;}

protected:
	std::string myName;
};

