#pragma once

#include "core/Color.h"
#include "core/Geometry.h"

#include <memory>

class Texture
{
public:
	typedef std::shared_ptr<Texture> ptr;
public:
	Texture();
	virtual ~Texture();

	virtual Color eval(const Point2d&) = 0;
};

