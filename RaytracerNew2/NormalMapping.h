#pragma once
#include "BSDF.h"

#include "Parameters.h"

class NormalMapping :
	public BSDF
{
public:
	NormalMapping(const Parameters& params);
	~NormalMapping();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;

	real pdf(const BSDFSamplingInfos& infos) override;

protected:
	DifferentialGeometry getFrame(const BSDFSamplingInfos & infos);
	BSDFSamplingInfos getPerturbedInfos(DifferentialGeometry perturbedShading, const BSDFSamplingInfos & infos);

protected:
	Texture::ptr myNormalMap;
	BSDF::ptr myBSDF;
};

