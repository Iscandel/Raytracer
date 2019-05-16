#pragma once
#include "BSDF.h"

#include "MicrofacetDistribution.h"

#include "tools/Rng.h"

class Parameters;

class LayeredBSDF :
	public BSDF
{
public:
	LayeredBSDF(const Parameters& params);
	~LayeredBSDF();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;

	real pdf(const BSDFSamplingInfos& infos) override;

	//=============================================================================
	///////////////////////////////////////////////////////////////////////////////
	Color fresnel(real etaExt, real etaInt, real cosThetaI, real& etaI, real& etaT, real& relativeEta, real& cosThetaT);

	Color evalReflection(const BSDFSamplingInfos & infos, const Color& fr, real alpha);

	BSDF::ptr myTopBSDF;
	BSDF::ptr myBaseBSDF;

	MicrofacetDistribution myDistribution;

	real myEtaExt;
	real myEtaInt;
	real myAlphaTop;
	real myAlphaBase;
	real myThickness;
	Color myAbsorbance;
	Rng myRng;

	Color myEta;
	Color myAbsorption;
};

