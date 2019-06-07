#include "NormalIntegrator.h"

#include "core/Intersection.h"
#include "factory/ObjectFactoryManager.h"
#include "core/Scene.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
NormalIntegrator::NormalIntegrator(const Parameters& params)
:Integrator(params)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
NormalIntegrator::~NormalIntegrator()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Color NormalIntegrator::li(Scene & scene, Sampler::ptr, const Ray & ray, RadianceType::ERadianceType)
{
	Intersection intersection;
	if (scene.computeIntersection(ray, intersection))
	{
		Normal3d n = intersection.myShadingGeometry.myN;
		return Color::fromRGB(n.cwiseAbs().x(), n.cwiseAbs().y(), n.cwiseAbs().z());
	}

	return Color();
}

RT_REGISTER_TYPE(NormalIntegrator, Integrator)