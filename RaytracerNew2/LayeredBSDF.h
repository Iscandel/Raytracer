#pragma once
#include "BSDF.h"

#include "MicrofacetDistribution.h"

#include "Rng.h"

class Parameters;

class LayeredBSDF :
	public BSDF
{
public:
	LayeredBSDF(const Parameters& params);
	~LayeredBSDF();

	Color eval(const BSDFSamplingInfos& infos) override;

	Color sample(BSDFSamplingInfos& infos, const Point2d& sample) override;

	double pdf(const BSDFSamplingInfos& infos) override;

	//=============================================================================
	///////////////////////////////////////////////////////////////////////////////
	Color fresnel(double etaExt, double etaInt, double cosThetaI, double& etaI, double& etaT, double& relativeEta, double& cosThetaT);

	Color evalReflection(const BSDFSamplingInfos & infos, const Color& fr, double alpha);

	BSDF::ptr myTopBSDF;
	BSDF::ptr myBaseBSDF;

	MicrofacetDistribution myDistribution;

	double myEtaExt;
	double myEtaInt;
	double myAlphaTop;
	double myAlphaBase;
	double myThickness;
	Color myAbsorbance;
	Rng myRng;

	Color myEta;
	Color myAbsorption;
};

