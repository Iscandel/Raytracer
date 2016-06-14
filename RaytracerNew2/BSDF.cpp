#include "BSDF.h"

#include "Intersection.h"

BSDF::BSDF()
{
}


BSDF::~BSDF()
{
}


BSDFSamplingInfos::BSDFSamplingInfos(const Intersection& inter, const Vector3d& _wi, const Vector3d& _wo)
:BSDFSamplingInfos(_wi, _wo)
{
	uv = inter.myUv;
	shadingFrame = inter.myShadingGeometry;
}

BSDFSamplingInfos::BSDFSamplingInfos(const Intersection & inter, const Vector3d & _wi)
:BSDFSamplingInfos(inter, _wi, Vector3d())
{
}
