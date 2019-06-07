#pragma once
#include "BSDF.h"

#include "core/Parameters.h"

class BumpMapping : public BSDF
{
public:
	BumpMapping(const Parameters& params);
	~BumpMapping();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;

	real pdf(const BSDFSamplingInfos& infos) override;

protected:
	DifferentialGeometry getFrame(const BSDFSamplingInfos & infos);
	BSDFSamplingInfos getPerturbedInfos(DifferentialGeometry perturbedShading, const BSDFSamplingInfos & infos);

protected:
	Texture::ptr myBumpMap;
	BSDF::ptr myBSDF;
};

