#include "BumpMapping.h"

#include "bsdf/Diffuse.h"
#include "factory/ObjectFactoryManager.h"
#include "texture/ConstantTexture.h"

BumpMapping::BumpMapping(const Parameters& params)
:BSDF(params)
{
	//Don't forget to put gamma == 1 in the scene file for the texture... !!!!!!!!!!!!!

	myBumpMap = params.getTexture("bumpMap", Texture::ptr(new ConstantTexture(Color(0.))));
	myBSDF = params.getBSDF("bsdf", BSDF::ptr(new Diffuse(Parameters())));
}


BumpMapping::~BumpMapping()
{
}

Color BumpMapping::eval(const BSDFSamplingInfos & infos)
{
	DifferentialGeometry perturbedShading = getFrame(infos);
	BSDFSamplingInfos perturbedInfos = getPerturbedInfos(perturbedShading, infos);

	if (DifferentialGeometry::cosTheta(infos.wo) * DifferentialGeometry::cosTheta(perturbedInfos.wo) <= 0.f)
		return Color(0.f);

	return myBSDF->eval(perturbedInfos);
}

DifferentialGeometry BumpMapping::getFrame(const BSDFSamplingInfos & infos)
{
	Color grad[2];
	myBumpMap->evalGradient(infos.uv, grad);
	real gradX = grad[0].luminance();
	real gradY = grad[1].luminance();

	Vector3d dpdu = infos.shadingFrame.toLocal(infos.shadingFrame.dpdu);
	Vector3d dpdv = infos.shadingFrame.toLocal(infos.shadingFrame.dpdv);
	Normal3d n(0, 0, 1);

	//From Blinn's paper: Simulation of wreckled surfaces
#ifdef USE_ALIGN
	Normal3d bumped = n + gradX * n.cross3(dpdv) - gradY * n.cross3(dpdu);
#else
	Normal3d bumped = n + gradX * n.cross(dpdv) - gradY * n.cross(dpdu);
#endif

	bumped.normalize();

	DifferentialGeometry perturbedShading = DifferentialGeometry(infos.shadingFrame.toWorld(bumped));

	return perturbedShading;
}

BSDFSamplingInfos BumpMapping::getPerturbedInfos(DifferentialGeometry perturbedShading, const BSDFSamplingInfos & infos)
{
	BSDFSamplingInfos perturbedInfos(
		perturbedShading.toLocal(infos.shadingFrame.toWorld(infos.wi)),
		perturbedShading.toLocal(infos.shadingFrame.toWorld(infos.wo))
	);
	perturbedInfos.uv = infos.uv;
	perturbedInfos.shadingFrame = perturbedShading;
	perturbedInfos.measure = infos.measure;

	return perturbedInfos;
}

Color BumpMapping::sample(BSDFSamplingInfos & infos, const Point2d & sample)
{
	DifferentialGeometry perturbedShading = getFrame(infos);
	BSDFSamplingInfos perturbedInfos = getPerturbedInfos(perturbedShading, infos);

	Color weight = myBSDF->sample(perturbedInfos, sample);
	infos.wo = infos.shadingFrame.toLocal(perturbedInfos.shadingFrame.toWorld(perturbedInfos.wo));

	if (DifferentialGeometry::cosTheta(infos.wo) * DifferentialGeometry::cosTheta(perturbedInfos.wo) <= 0.f)
		return Color(0.f);

	infos.measure = perturbedInfos.measure;
	infos.pdf = perturbedInfos.pdf;
	infos.relativeEta = perturbedInfos.relativeEta;
	infos.sampledType = perturbedInfos.sampledType;

	return weight;
}

real BumpMapping::pdf(const BSDFSamplingInfos & infos)
{
	DifferentialGeometry perturbedShading = getFrame(infos);
	BSDFSamplingInfos perturbedInfos = getPerturbedInfos(perturbedShading, infos);

	if (DifferentialGeometry::cosTheta(infos.wo) * DifferentialGeometry::cosTheta(perturbedInfos.wo) <= 0.f)
		return (real)0;

	return myBSDF->pdf(perturbedInfos);
}

RT_REGISTER_TYPE(BumpMapping, BSDF)