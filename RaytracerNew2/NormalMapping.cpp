#include "NormalMapping.h"

#include "ConstantTexture.h"
#include "DifferentialGeometry.h"
#include "Diffuse.h"
#include "ObjectFactoryManager.h"

NormalMapping::NormalMapping(const Parameters& params)
{
	myNormalMap = params.getTexture("normalMap", Texture::ptr(new ConstantTexture(Color(0.))));
	myBSDF = params.getBSDF("bsdf", BSDF::ptr(new Diffuse(Parameters())));
}


NormalMapping::~NormalMapping()
{
}

Color NormalMapping::eval(const BSDFSamplingInfos & infos)
{
	//Color tmp = myNormalMap->eval(infos.uv);
	//Normal3d normal(tmp.r, tmp.g, tmp.b);
	//for (int i = 0; i < 3; i++)
	//	normal[i] = 2 * normal[i] - 1.;
	//
	//DifferentialGeometry perturbedShading = DifferentialGeometry(infos.shadingFrame.toWorld(normal));

	DifferentialGeometry perturbedShading = getFrame(infos);
	BSDFSamplingInfos perturbedInfos = getPerturbedInfos(perturbedShading, infos);

	return myBSDF->eval(perturbedInfos);
}

DifferentialGeometry NormalMapping::getFrame(const BSDFSamplingInfos & infos)
{
	Color tmp = myNormalMap->eval(infos.uv);
	Normal3d normal(tmp.r, tmp.g, tmp.b);
	for (int i = 0; i < 3; i++)
		normal[i] = 2 * normal[i] - 1.;

	DifferentialGeometry perturbedShading = DifferentialGeometry(infos.shadingFrame.toWorld(normal));

	return perturbedShading;
}

BSDFSamplingInfos NormalMapping::getPerturbedInfos(DifferentialGeometry perturbedShading, const BSDFSamplingInfos & infos)
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

Color NormalMapping::sample(BSDFSamplingInfos & infos, const Point2d & sample)
{
	DifferentialGeometry perturbedShading = getFrame(infos);
	BSDFSamplingInfos perturbedInfos = getPerturbedInfos(perturbedShading, infos);

	return myBSDF->sample(perturbedInfos, sample);
}

double NormalMapping::pdf(const BSDFSamplingInfos & infos)
{
	DifferentialGeometry perturbedShading = getFrame(infos);
	BSDFSamplingInfos perturbedInfos = getPerturbedInfos(perturbedShading, infos);
	return myBSDF->pdf(perturbedInfos);
}

RT_REGISTER_TYPE(NormalMapping, BSDF)