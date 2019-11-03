#pragma once

#include "core/Color.h"
#include "core/Geometry.h"
#include "core/RaytracerObject.h"

#include <memory>

class Texture : RaytracerObject
{
public:
	enum BoundaryCondition {
		REPEAT,
		MIRROR
	};

public:
	typedef std::shared_ptr<Texture> ptr;
public:
	Texture(const Parameters& params);
	virtual ~Texture();

	virtual Color eval(const Point2d&) = 0;
	virtual Color getAverage() const = 0;
	virtual Color getMin() const = 0;
	virtual Color getMax() const = 0;

protected:
	BoundaryCondition parseBoundaryCondition(const std::string& condition);

protected:
	BoundaryCondition myBoundaryCondition;
};

