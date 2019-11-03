#include "Texture.h"

#include "core/Parameters.h"

Texture::Texture(const Parameters& params)
:RaytracerObject(params)
{
}


Texture::~Texture()
{
}

Texture::BoundaryCondition Texture::parseBoundaryCondition(const std::string& condition)
{
	if (condition == "repeat")
		return Texture::REPEAT;
	else if (condition == "mirror")
		return Texture::MIRROR;

	return Texture::REPEAT;
}