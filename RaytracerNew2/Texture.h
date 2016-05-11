#pragma once

#include "Color.h"
#include "Geometry.h"

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

