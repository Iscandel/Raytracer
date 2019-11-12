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

void Texture::evalGradient(const Point2d& uv, Color gradient[2])
{
	//According to Taylor development, 
	//f(u+e) - f(u-e) = f(u,v) + e df/du - (f(u,v) - e df/du)
	//f(u+e) - f(u-e) = 2e df/du
	//(f(u+e) - f(u-e)) / 2e = df/du
	//Also possible:
	//f(u+e) - f(u) = e df/du
	//(f(u+e) - f(u)) / e = df/du
	//Or Prewitt / Sobel... filters

	const real e = math::EPSILON;
	Point2d uDirPos(uv.x() + e, uv.y());
	Point2d uDirNeg(uv.x() - e, uv.y());
	Point2d vDirPos(uv.x(), uv.y() + e);
	Point2d vDirNeg(uv.x(), uv.y() - e);
	
	gradient[0] = (eval(uDirPos) - eval(uDirNeg)) / (2 * e);
	gradient[1] = (eval(vDirPos) - eval(vDirNeg)) / (2 * e);
}