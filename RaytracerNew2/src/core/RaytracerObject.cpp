#include "RaytracerObject.h"

#include "Parameters.h"

RaytracerObject::RaytracerObject(const Parameters& params)
{
	myParameters = std::make_unique<Parameters>(params);
}


RaytracerObject::~RaytracerObject()
{
}
